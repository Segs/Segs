#include "GroupDraw.h"

#include "graphics/gfx.h"
#include "entity/EntClient.h"
#include "GameState.h"
#include "utils/dll_patcher.h"
#include "renderer/RenderTricks.h"
#include "renderer/RenderTree.h"
#include "renderer/ModelCache.h"
#include "renderer/RendererState.h"
#include "renderer/Model.h"
#include "renderer/RendererUtils.h"

#include <cmath>
#include <cstring>

struct Grid
{
    void *cells;
    Vector3 pos;
    float size;
    float size_inv;
    uint32_t tag;
    uint32_t bitcount;
    int valid_id;
    MemPool *entries_pool;
    MemPool *child_ptrs_pool;
    MemPool *cells_pool;
    MemPool *idx_lists_pool;
};

static_assert (sizeof(Grid)==0x34, "sizeof(Grid)==0x34");

extern "C" {
    __declspec(dllimport) char *fn_4C6050(GroupDef *grp, const char *name); //groupDefFindPropertyValue
    __declspec(dllimport) char *strstri(const char *str1, const char *str2);
    __declspec(dllimport) DefTracker *groupFindInside(Vector3 *arg0, int find_type);
    __declspec(dllimport) void fn_415B90(Matrix4x3 *mat, float rad, int nsegs, unsigned int color); //drawSphere
    __declspec(dllimport) bool legitBone(int);
    __declspec(dllimport) void gfxTreeFindWorldSpaceMat(Matrix4x3 *, GfxTree_Node *);
    __declspec(dllimport) void skySetIndoorFogVals(int idx, float start, float end, RGBA *clrs);
    __declspec(dllimport) void fn_5B6740(float yaw, Matrix3x3 *rot_mat);//yawMat3
    __declspec(dllimport) void  pitchMat3(float pitch_angle, Matrix3x3 *tgt);
    __declspec(dllimport) void  fn_5B6840(float roll_angle, Matrix3x3 *tgt); //rollMat3
    __declspec(dllimport) Handle fxManageWorldFx(Handle result, const char *, Matrix4x3 *, float vis_dist, float lod_far_fade, DefTracker *);
    __declspec(dllimport) void lightTracker(DefTracker *);
    __declspec(dllimport) void trackerOpen(DefTracker *grp);
    __declspec(dllimport) void portalOpen(DefTracker *);
    __declspec(dllimport) bool inEditMode();
    __declspec(dllimport) void checkForVisScaleChange(float);
    __declspec(dllimport) void initSwayTable();
    __declspec(dllimport) int selDraw();
    __declspec(dllimport) void *gridFindCell(Grid *grid, Vector3 *pos, DefTracker **obj_list, int *count_ptr, float *size_ptr);

    __declspec(dllimport) float fog_far_dist;
    __declspec(dllimport) float g_curr_fog_dist;
    __declspec(dllimport) int g_see_everything;
    __declspec(dllimport) int see_outside;
    __declspec(dllimport) int draw_id;
    __declspec(dllimport) CameraInfo cam_info;
    __declspec(dllimport) int entities_max;
    __declspec(dllimport) char entity_slot_allocated[10240];
    __declspec(dllimport) Entity *entities_arr[10240];
    __declspec(dllimport) float float_7B8890[256];
    __declspec(dllimport) EditCommandState g_edit_state;
    __declspec(dllimport) GroupInfo *group_info_0;
    __declspec(dllimport) GroupInfo group_info;
    __declspec(dllimport) float g_TIMESTEP;
    __declspec(dllimport) GfxWindow g_frustumdata;
    __declspec(dllimport) Parsed_SkyScene parsed_scene;
}
void lightModel(Model *model, Matrix4x3 *mat, uint8_t *rgb, DefTracker *light_trkr);

int camera_is_inside;
static EntLight *g_fx_light;
static FxMiniTracker *g_fx_mini_tracker;
static float sway_count;
struct DefDrawContext
{
    int ref_idx=0;
    int depth=0;
};
std::vector<DefTracker *> visibleTrays;
static int shadowVolumeVisible(Vector3 *start_point, float step, float length_to_check)
{
    if (g_State.view.bShadowVol == 2)
        return 0;
    Vector3 delta = g_sun.shadow_direction_in_viewspace * step;
    Vector3 center = delta + *start_point;
    for (float i = 0.0; i < length_to_check; i = i + step)
    {
        if (gfxSphereVisible(&center, step))
            return 1;
        center += delta;
    }
    return 0;
}
static void makeAGroupEnt(GroupEnt &to_init, GroupDef *grpdef, const Matrix4x3 *transform, float draw_dist)
{
    to_init.mid_cache  = *transform * grpdef->mid_;
    to_init.transform = transform;
    to_init.m_def = grpdef;
    to_init.vist_dist_cache = draw_dist * grpdef->vis_dist + grpdef->radius;
    float dist = grpdef->lod_scale * grpdef->vis_dist * draw_dist + grpdef->radius;
    to_init.vis_dist_sqr_cache = dist * dist;
    to_init.flags_cache |= (grpdef->parent_fade << 0);
    to_init.flags_cache |= (grpdef->vis_doorframe << 1);
    to_init.flags_cache |= (grpdef->child_parent_fade<<2);
    to_init.flags_cache |= (grpdef->has_fog<<4) | 8;
}
bool extractedVisibilityTest(GroupDef *def, Vector3 &pos, float dist)
{
    if (!g_see_everything)
    {
        if (def->has_fog)
        {
            if (dist > def->fog_radius * def->fog_radius || dist > g_curr_fog_dist)
                return false;
            g_curr_fog_dist = dist;
            //TODO: check conversion here!
            skySetIndoorFogVals(0, def->fog_near, def->fog_far, (RGBA *)def->tint_colors);
            skySetIndoorFogVals(1, def->fog_near, def->fog_far, (RGBA *)&def->tint_colors[1]);
        }
        if (def->has_sound || def->has_fog || def->has_light)
            return false;
        if (def->model && (def->model->trck_node && def->model->trck_node->_TrickFlags & 0x200000))
        {
            return false;
        }
    }
    if (fog_far_dist != 0.0f && (fog_far_dist + def->radius) * (fog_far_dist + def->radius) < dist)
        return false;

    if (def->model)
    {
        if (gfxSphereVisible(&pos, def->radius) == 0)
        {
            if (def->shadow_dist == 0.0f || !shadowVolumeVisible(&pos, def->radius, def->shadow_dist))
                return false;
        }
    }
    else if (gfxSphereVisible(&pos, def->shadow_dist + def->radius) == 0)
        return false;
    return true;
}
int modelCalcAlpha(Vector3 *mid, GroupDef *def, float lod_scale)
{

    Model *model = def->model;
    float lod_near = lod_scale * def->lod_near;
    float lod_near_fade = lod_scale * def->lod_near_fade;
    float lod_far = lod_scale * def->lod_far;
    float lod_far_fade = lod_scale * def->lod_far_fade;
    float scale_a = mid->length();
    assert(model);
    if (scale_a < 50.0f)
    {
        if (model->Model_flg1 & OBJ_TREE)
            return 254;
    }
    if (lod_near - 0.5f > scale_a)
    {
        if (lod_near - scale_a >= lod_near_fade)
            return 0;

        float ratio = (lod_near - scale_a) / lod_near_fade;
        return int32_t((1.0f - ratio) * 255.0f);
    }
    if (scale_a >lod_far)
    {
        if (def->window)
            return 255;

        if (scale_a - lod_far < lod_far_fade)
        {
            float ratio = (scale_a - lod_far) / lod_far_fade;
            return int32_t((1.0f - ratio) * 255.0f);
        }
        return 0;
    }
    if (!def->window)
        return 255;
    return int32_t(std::max(80.0f, scale_a / lod_far * 255.0f));
}
static bool groupDefIsDoorGenerator(GroupDef *def)
{
    if (!def->groupprop_rel_map)
        return false;
    char *generatedType = fn_4C6050(def, "Generator");
    return generatedType && strstri(generatedType, "Door");
}
static Entity *entFindEntAtThisPosition(Vector3 *vec1, float eps)
{
    for (int i = 1; i < entities_max; ++i)
    {
        if (entity_slot_allocated[i] & 1 && isVectorDistanceLessThenEps(vec1, &entities_arr[i]->mat4.TranslationPart, eps))
            return entities_arr[i];
    }
    return nullptr;
}
int segs_seqgraphics_getBoneNodes(GfxTree_Node *node, GfxTree_Node **all_nodes, Handle seq_handle)
{
    int found_count = 0;
    for( ;node; node=node->next)
    {
        if (node->seqHandle != seq_handle)
            continue;

        if (legitBone(node->anim_id()) && node->model && node->model->Model_flg1 & OBJ_STATICFX)
        {
            all_nodes[node->anim_id()] = node;
            ++found_count;
        }
        if (node->children_list)
            found_count += segs_seqgraphics_getBoneNodes(node->children_list, all_nodes, seq_handle);
    }
    return found_count;
}
int seqSetStaticLight(SeqInstance *arg0, DefTracker *light_trkr)
{
    char buf[100000];
    GfxTree_Node *bones[70];
    Matrix4x3 operator_mat;

    if (!arg0 || !arg0->gfx_root || !arg0->gfx_root->children_list)
        return 1;
    memset(bones, 0, sizeof(GfxTree_Node *)*70);
    if (0==segs_seqgraphics_getBoneNodes(arg0->gfx_root->children_list, bones, arg0->seq_instance_handle))
        return 1;
    if (!light_trkr)
        light_trkr = groupFindInside(&arg0->gfx_root->mat.TranslationPart, 2);
    if (!light_trkr || !light_trkr->light_grid)
        return 0;
    for (int i = 0; i < 70; ++i)
    {
        if (!bones[i])
            continue;
        assert(bones[i]->anim_id() == i && !bones[i]->rgb_entries && legitBone(i) && bones[i]->model && bones[i]->model->Model_flg1 & 0x8000); //OBJ_STATICFX
        gfxTreeFindWorldSpaceMat(&operator_mat, bones[i]);
        size_t size = 4 * bones[i]->model->vertex_count;
        char *vertices_memory = buf;
        if (size > 100000)
        {
            vertices_memory = (char *)COH_MALLOC(size);
        }
        lightModel(bones[i]->model, &operator_mat, (unsigned __int8 *)vertices_memory, light_trkr);
        arg0->bone_rgb_buffers[i] = segs_modelConvertRgbBuffer(vertices_memory, bones[i]->model->vertex_count, vertices_memory != buf);
        bones[i]->rgb_entries = arg0->bone_rgb_buffers[i];
    }
    return 1;
}
static void lightGiveLightTrackerToMyDoor(DefTracker *tracker, DefTracker *light_trkr)
{
    assert(tracker && light_trkr);
    Entity *ent = entFindEntAtThisPosition(&tracker->matrix1.TranslationPart, 0.1f);
    if (ent && ent->seq && ent->seq->static_light_done != 1)
    {
        if (seqSetStaticLight(ent->seq, light_trkr))
            ent->seq->static_light_done = 1;
    }
}

void lightModel(Model *model, Matrix4x3 *mat, unsigned __int8 *rgb, DefTracker *light_trkr)
{
    int j;
    Vector3 normal_in_ws;
    Vector3 primary_color;
    uint8_t max_light_colors[3];
    DefTracker *found_trackers[1001];

    int is_omnidir_light = 0;
    int tracker_count = 0;
    Vector3 masked_dst = { -8.0000002e16f, -8.0000002e16f, -8.0000002e16f};
    float cell_power_of_two_size = 0.0;
    assert(rgb && light_trkr);
    Grid *light_grid = light_trkr->light_grid;
    if (!light_grid)
        return;

    assert(model && model->vbo);
    segs_modelSetupVertexObject(model, 1);
    Vector3 *verts_pos = (Vector3 *)model->vbo->cpuside_memory;
    Vector3 *verts_normal = (Vector3 *)(intptr_t(model->vbo->normals_offset) + (char *)model->vbo->cpuside_memory);
    for (int i = 0; i < 3; ++i)
    {
        primary_color[i] = ((uint8_t *)light_trkr->def->tint_colors)[i] / 255.0f;
        max_light_colors[i] = *((char *)&light_trkr->def->tint_colors[1] + i);
    }
    if (!(max_light_colors[2] | max_light_colors[1] | max_light_colors[0]))
    {
        max_light_colors[2] = 0xFFu;
        max_light_colors[1] = 0xFFu;
        max_light_colors[0] = 0xFFu;
    }
    if (strstri(model->bone_name_offset, "omni"))
        is_omnidir_light = 1;
    for (int i = 0; i < model->vertex_count; ++i)
    {
        Vector3 dst    = *mat * verts_pos[i];
        Vector3 calc_color = primary_color;
        for (j = 0; j < 3; ++j)
        {
            float tmp = dst[j] - masked_dst[j];
            if (tmp >= cell_power_of_two_size || tmp < 0.0f)
                break;
        }
        if (j < 3)
        {
            ++light_grid->tag;
            gridFindCell(light_grid, &dst, found_trackers, &tracker_count, &cell_power_of_two_size);
            cell_power_of_two_size = std::min(128.0f, cell_power_of_two_size);
            for (int k = 0; k < 3; ++k)
            {
                int32_t int_dest = int(dst[k]);
                masked_dst[k] = (~(int(cell_power_of_two_size) - 1) & int_dest);
            }
        }
        for (int l = 0; l < tracker_count; ++l)
        {
            DefTracker *trkr = found_trackers[l];
            GroupDef *def = trkr->def;
            if (!def || def->key_light)
                continue;
            Vector3 vec_to_light = dst - trkr->matrix1.TranslationPart;
            float dist_to_light = vec_to_light.normalize();
            if ((is_omnidir_light && dist_to_light > 3.0f) || dist_to_light > def->light_radius)
                continue;
            multVertexByMatrix(&verts_normal[i], &mat->ref3(), &normal_in_ws);
            float light_magnitude = -vec_to_light.dot(vec_to_light);
            if (model->Model_flg1 & OBJ_NOLIGHTANGLE)
                light_magnitude = 1.0;
            if (light_magnitude < 0.0f)
                continue;
            float light_falloff_radius = def->light_radius / 3.0f;
            float light_mult = 1.0;
            if (dist_to_light >= light_falloff_radius)
            {
                float tmp = (def->light_radius - dist_to_light) / (def->light_radius - light_falloff_radius);
                light_mult = tmp * tmp;
            }
            light_mult = light_mult * light_magnitude;
            if (light_mult >= 0.05f)
            {
                light_mult   = light_mult / 255.0f;
                vec_to_light.x = (float)uint8_t(def->tint_colors[0]) * light_mult;
                vec_to_light.y = (float)uint8_t(def->tint_colors[0]>>8) * light_mult;
                vec_to_light.z = (float)uint8_t(def->tint_colors[0]>>16) * light_mult;
                if (def->tint_colors[0] & 0x1000000) //TODO: this is a strange way of marking a light as brightening/darkening one
                    calc_color -= vec_to_light;
                else
                    calc_color += vec_to_light;
            }
        }
        // clamp the calculated color to max_light_colors
        for (int m = 0; m < 3; ++m)
        {
            *rgb++ = uint8_t(clamp<int>(uint8_t(calc_color[m] * 255.0f), 0, max_light_colors[m]));
        }
        *rgb++ = 0xFFu;
    }
}
static void colorizeTracker(Matrix4x3 *mat, DefTracker *tracker, DefTracker *light_trkr)
{
    Matrix4x3 operator_mat;
    uint8_t rgb[100000];

    if (!tracker || !tracker->def->model)
        return;

    size_t colorization_buf_size = 4 * tracker->def->model->vertex_count;
    uint8_t *data_tgt = rgb;
    if (colorization_buf_size > 100000)
    {
        data_tgt = (uint8_t*)COH_MALLOC(colorization_buf_size);
    }
    operator_mat = cam_info.inv_viewmat * *mat;
    lightModel(tracker->def->model, &operator_mat, data_tgt, light_trkr);
    tracker->RGB_VBO = segs_modelConvertRgbBuffer(data_tgt, tracker->def->model->vertex_count, data_tgt != rgb);
}
void drawDefSub(GroupDef *defc, DrawParams *draw_params, Matrix4x3 *mat, Vector3 *mid, DefTracker *tracker)
{
    int alpha;
    MaterialDefinition inital_material(g_default_mat);
    inital_material.setDrawMode(DrawMode::SINGLETEX);
    inital_material.setFragmentMode(eBlendMode::MULTIPLY);
    Model *model = defc->model;
    assert(model);
    if (!(model->loadstate & 4))
        return;
    assert(model->vbo);
    if (model->model_tri_count < g_State.tri_cutoff || (g_State.tri_cutoff < 0 && model->model_tri_count > -g_State.tri_cutoff))
        return;

    TrickNode *trick_node = model->trck_node;
    if (trick_node && (trick_node->_TrickFlags & NoDraw || (!g_see_everything && trick_node->_TrickFlags & EditorVisible)))
        return;
    if (defc->parent_fade && draw_params->node_mid.z != 0.0f)
        alpha = modelCalcAlpha(&draw_params->node_mid, defc, draw_params->scale);
    else
        alpha = modelCalcAlpha(mid, defc, draw_params->scale);
    if (!alpha)
        return;
    GLuint rgb_vbo = 0;
    if (tracker)
    {
        if (tracker->flags & 2)
        {
            if (tracker->instance_mods && !(tracker->instance_mods->_TrickFlags & NoDraw))
                segs_modelDraw(model, mat, tracker->instance_mods, alpha, 0, nullptr,inital_material);

            return;
        }
        if (draw_params->pGrp)
        {
            if (!tracker->RGB_VBO)
            {
                assert(tracker->def->model);
                DefTracker *parent_group = tracker->parent_group;
                if (parent_group && groupDefIsDoorGenerator(parent_group->def))
                    lightGiveLightTrackerToMyDoor(tracker->parent_group, draw_params->pGrp);
                Model *model = tracker->def->model;
                if (!(model->Model_flg1 & OBJ_FULLBRIGHT))
                {
                    if (!model->trck_node || !(model->trck_node->_TrickFlags & EditorVisible))
                        colorizeTracker(mat, tracker, draw_params->pGrp);
                }
            }
            rgb_vbo = tracker->RGB_VBO;
        }
    }
    if (model->trck_node && model->trck_node->_TrickFlags & CastShadow)
    {
        if (g_sun.shadowcolor.w >= 0.05f)
            segs_modelAddShadow(mat, alpha, model, 0x40);
    }
    else
    {
        segs_addViewSortNode(nullptr, model, mat, mid, alpha, rgb_vbo, (RGBA *)draw_params->tint_color_ptr, draw_params->tex_binds, g_fx_light);
    }
}
static void swayThisMat(Model *model, Matrix4x3 *tgt)
{
    float scale = model->trck_node->info->Sway_Rotate;
    float amount = sway_count * model->trck_node->info->Sway;
    if (scale == 0.0f)
    {
        fn_5B6740(amount, &tgt->ref3());
    }
    else
    {
        int idx = int32_t(amount);
        float pitch = lerp(float_7B8890[idx & 0xFF], float_7B8890[(idx+1) & 0xFF], amount - idx) * scale;
        idx = int32_t(amount * 1.5f);
        float roll = lerp(float_7B8890[idx & 0xFF], float_7B8890[(idx + 1) & 0xFF], amount * 1.5f - idx) * scale;
        pitchMat3(pitch, &tgt->ref3());
        fn_5B6840(roll, &tgt->ref3());
    }
}
void addToVisibleTraysAndDraw(DefTracker* p_tracker, GroupDef* group, Matrix4x3 &mat, Vector3 &mid, DrawParams &draw)
{
    if (group->has_tint_color)
        draw.tint_color_ptr = group->tint_colors;
    if (group->has_tex)
    {
        if (group->has_tex & 1)
            draw.tex_binds[0] = group->textures_arr[0];
        if (group->has_tex & 2)
            draw.tex_binds[1] = group->textures_arr[1];
    }

    if (group->tray)
        visibleTrays.push_back(p_tracker);

    if (group->model)
    {
        if (group->model->trck_node && group->model->trck_node->info && group->model->trck_node->info->Sway != 0.0f)
        {
            swayThisMat(group->model, &mat);
        }
        drawDefSub(group, &draw, &mat, &mid, p_tracker);
    }
    if (group->lod_fadenode)
        draw.node_mid = mid;
    if (g_State.can_edit && group->beacon_radius != 0.0f && g_edit_state.beaconradii)
    {
        Matrix4x3 combined = cam_info.inv_viewmat * mat;
        fn_415B90(&combined, group->beacon_radius, 11, 0xFFFFu);
    }
}

DefTracker* doOpen(DefTracker *p_tracker, GroupDef *grp, DrawParams &draw, float lod_rel, int vis)
{
    DefTracker* res = nullptr;
    if (!p_tracker)
        return res;
    if (!(p_tracker->flags & 2))
    {
        if (grp->has_ambient)
        {
            if (!p_tracker->light_grid)
                lightTracker(p_tracker);
            draw.pGrp = p_tracker;
        }
        if (!p_tracker->subgroups && (draw.pGrp || p_tracker->def->open_tracker))
            trackerOpen(p_tracker);
        if (grp->tray && !(p_tracker->flags & 8))
            portalOpen(p_tracker);
        if (p_tracker->subgroups)
            res = p_tracker->subgroups;
        if (grp->has_fx)
        {
            float lod_far_fade = lod_rel * grp->lod_far_fade;
            float vis_dist = lod_rel * grp->vis_dist;
            p_tracker->handle = fxManageWorldFx(p_tracker->handle, grp->def_type, &p_tracker->matrix1, vis_dist, lod_far_fade, draw.pGrp);
        }
    }
    if (grp->outside)
        see_outside = 1;
    if (p_tracker->portal_groups && vis != 3)
        p_tracker->draw_id = draw_id;
    return res;
}
void segs_drawDefInternal(GroupEnt &arg0, const Matrix4x3 &parent_mat, DefTracker *p_tracker, int vis, DrawParams *draw, bool flag, DefDrawContext &ctx);
void recursePortals(DefTracker *tracker, GroupDef *def, DrawParams &draw_self, DrawParams *draw_root, int lod_rela, DefDrawContext &ctx)
{
    if (!tracker || !tracker->portal_groups || lod_rela == 3)
        return;
    assert(lod_rela != 4);

    if ((def->vis_blocker & 1) == 2)
        assert(0);
    if (def->vis_blocker & 1 && lod_rela != 1)
        return;
    ModelExtra *portals = tracker->def->model->extra;
    draw_self.tint_color_ptr = nullptr;
    for (int idx = 0; idx < portals->portal_count; ++idx)
    {
        DefTracker *portal = tracker->portal_groups[idx];
        if (!portal)
            continue;
        GroupEnt tmp_grp;
        if(portal->draw_id == draw_id && (portal->def->vis_blocker & 2)==0)
            continue;
        makeAGroupEnt(tmp_grp, portal->def, &portal->matrix1, draw_root->scale);
        segs_drawDefInternal(tmp_grp, cam_info.viewmat, portal, 2, draw_root, portal->draw_id == draw_id, ctx);
    }
}
static void recurseSubs(DefTracker *tracker, DefTracker *sub_trackers, GroupDef *grp, DrawParams &draw_dist, const Matrix4x3 &mat, float dist_non_sq, int lod_rela, DefDrawContext &ctx)
{
    int mode = lod_rela==3 ? 3 : 0;
    float dist = std::sqrt(dist_non_sq) - grp->radius;
    for (int idx_rz = 0; idx_rz < grp->num_subs; ++idx_rz)
    {
        GroupEnt & subgrp(grp->subdefs[idx_rz]);
        if (!(subgrp.flags_cache & 8)) {
            makeAGroupEnt(subgrp, subgrp.m_def, subgrp.transform, draw_dist.scale);
        }
        if (dist < subgrp.vist_dist_cache || subgrp.flags_cache & 0x10)
        {
            DefTracker *subgrp_tracker = sub_trackers ? &sub_trackers[idx_rz] : tracker;
            segs_drawDefInternal(subgrp, mat, subgrp_tracker, mode, &draw_dist, false, ctx);
        }
    }
}
static void handleGlobFxMiniTracker(Matrix4x3 *parent_mat, GroupDef *def, FxMiniTracker *globFxMiniTracker, float draw_scale)
{
    if (globFxMiniTracker->count<5)
    {
        Matrix4x3 world_mat          = cam_info.inv_viewmat * *parent_mat;
        float lod_far_fade = draw_scale * def->lod_far_fade;
        float vis_dist = draw_scale * def->vis_dist;
        globFxMiniTracker->fxIds[globFxMiniTracker->count] =
            fxManageWorldFx(globFxMiniTracker->fxIds[globFxMiniTracker->count], def->def_type, &world_mat, vis_dist,
                            lod_far_fade, nullptr);
        globFxMiniTracker->count ++;
    }
}
bool pre_test(GroupEnt &grp,DrawParams *draw,const Matrix4x3 &parent_mat,Vector3 &local_mid,float &mid_nonsq_length,DrawParams &tgtdraw)
{
    local_mid = parent_mat * grp.mid_cache;
    if (grp.flags_cache & 1 && draw->node_mid.z != 0.0f)
        mid_nonsq_length = draw->node_mid.lengthNonSqrt();
    else
        mid_nonsq_length = local_mid.lengthNonSqrt();
    if (mid_nonsq_length > grp.vis_dist_sqr_cache && !(grp.flags_cache & 0x16))
        return false;
    if (!extractedVisibilityTest(grp.m_def, local_mid, mid_nonsq_length))
        return false;
    float combined_lodscale = grp.m_def->lod_scale * draw->scale;
    float scaled_near = combined_lodscale * grp.m_def->lod_near;
    if (mid_nonsq_length < scaled_near * scaled_near)
        return false;
    tgtdraw  = *draw;
    tgtdraw.scale = combined_lodscale;

    return true;
}
void segs_drawDefInternal(GroupEnt &grp, const Matrix4x3 &parent_mat, DefTracker *p_tracker, int vis, DrawParams *draw,
                          bool flag, DefDrawContext &ctx)
{
    float mid_nonsq_length;
    Matrix4x3 combined_transform = Unity_Matrix;
    DrawParams draw_dist;
    DrawParams tmp_draw;
    bool isbuild = false;
    if(grp.m_def)
    {
        isbuild = nullptr != strstr(grp.m_def->ref_name, "_deco3_base");
        if (isbuild)
        {
            for (int i = 0; i < ctx.depth; ++i)
                printfDebug("  ");
            printfDebug("DDI: %s,%s,%s :", grp.m_def->ref_name, grp.m_def->dir, grp.m_def->def_type);
        }
    }
    Vector3 local_mid;
    if (!pre_test(grp, draw, parent_mat, local_mid, mid_nonsq_length, tmp_draw))
    {
        if(isbuild)
            printfDebug("F\n");
        return;
    }
    if (isbuild)
        printfDebug("T\n");

    GroupDef *mdef = grp.m_def;
    const Matrix4x3 *grp_transform = grp.transform;
    float def_and_draw_scale = mdef->lod_scale * draw->scale;

    if (mdef->tray && !vis)
    {
        if (!mdef->outside)
            return;
        vis = 2;
    }
    if (grp.flags_cache & 8)
    {
        combined_transform = parent_mat *  *grp_transform;
    }
    if (p_tracker && ((g_State.can_edit && p_tracker->instance_mods != nullptr && p_tracker->instance_mods->_TrickFlags & 0x2000) || p_tracker->flags & 0x20))
        return;
    if (!flag)
    {
        DefTracker *sub_trackers = doOpen(p_tracker, mdef, tmp_draw, def_and_draw_scale, vis);
        if (g_fx_mini_tracker && !p_tracker && mdef->has_fx)
            handleGlobFxMiniTracker(&combined_transform, mdef, g_fx_mini_tracker, def_and_draw_scale);
        addToVisibleTraysAndDraw(p_tracker, mdef, combined_transform, local_mid, tmp_draw);

        int def_num_subs = mdef->num_subs;
        int   init_mode = vis==3 ? 3 : 0;
        float rad_diff = std::sqrt(mid_nonsq_length) - mdef->radius;
        Matrix4x3 sub_transform;
        for (int idx_sub = 0; idx_sub < def_num_subs; ++idx_sub)
        {
            GroupEnt &subdef(mdef->subdefs[idx_sub]);
            DefTracker *tracker = sub_trackers ? &sub_trackers[idx_sub] : p_tracker;
            float sub_mid_nonsq_length;
            GroupDef *sub_group = subdef.m_def;

            if (!(subdef.flags_cache & 8))
                makeAGroupEnt(subdef, subdef.m_def, subdef.transform, tmp_draw.scale);
            if (rad_diff >= subdef.vist_dist_cache && !(subdef.flags_cache & 0x10))
                continue;
            int lod_rela = init_mode;
            Vector3 sub_midpoint;
            if(false==pre_test(subdef,&tmp_draw,combined_transform,sub_midpoint,sub_mid_nonsq_length,draw_dist))
                continue;
            float combined_lod_scale = tmp_draw.scale * sub_group->lod_scale;

            if (subdef.m_def->tray && !lod_rela) {
                if (!subdef.m_def->outside)
                    continue;
                lod_rela = 2;
            }
            if (subdef.flags_cache & 8) {
                sub_transform = combined_transform * *subdef.transform;
            }
            DefTracker *subgroup_trackers = nullptr;
            if (tracker) {
                if (g_State.can_edit && tracker->instance_mods && tracker->instance_mods->_TrickFlags & 0x2000)
                    continue;
                if (tracker->flags & 0x20)
                    continue;
                subgroup_trackers = doOpen(tracker, sub_group, draw_dist, combined_lod_scale, lod_rela);
            }
            if (g_fx_mini_tracker && !tracker && sub_group->has_fx) {
                handleGlobFxMiniTracker(&sub_transform, sub_group, g_fx_mini_tracker, combined_lod_scale);
            }
            addToVisibleTraysAndDraw(tracker, sub_group, sub_transform, sub_midpoint, draw_dist);
            ctx.depth++;
            recurseSubs(tracker,subgroup_trackers,sub_group,draw_dist,sub_transform,sub_mid_nonsq_length,lod_rela,ctx);
            recursePortals(tracker, sub_group, draw_dist, &tmp_draw, lod_rela,ctx);
            ctx.depth--;
        }
    }
    if (!p_tracker || !p_tracker->portal_groups || vis == 3)
        return;
    if ((mdef->vis_blocker & 1) == 2)
        assert(0);
    if (vis == 4)
        assert(0);
    if (mdef->vis_blocker & 1 && vis != 1)
        return;
    tmp_draw.tint_color_ptr = nullptr;

    ModelExtra *portals = p_tracker->def->model->extra;
    Matrix4x3 sub_transform;
    Vector3 portal_local_mid;
    GroupEnt tmp_group_ent;
    ctx.depth++;
    for (int idx = 0; idx<portals->portal_count; idx++)
    {
        DefTracker *portal_grp = p_tracker->portal_groups[idx];
        bool skip_open = false;
        if (!portal_grp)
            continue;
        if (portal_grp->draw_id == draw_id)
        {
            if ((portal_grp->def->vis_blocker & GroupDef::fAngleBlocker)!=GroupDef::fAngleBlocker)
                continue;
            skip_open = true;
        }
        makeAGroupEnt(tmp_group_ent, portal_grp->def, &portal_grp->matrix1, draw->scale);
        float portal_mid_nonsq_length;
        if(false==pre_test(tmp_group_ent,draw,cam_info.viewmat,portal_local_mid,portal_mid_nonsq_length,draw_dist))
            continue;
        def_and_draw_scale = tmp_group_ent.m_def->lod_scale * draw->scale;
        if (tmp_group_ent.flags_cache & 8)
        {
            sub_transform = cam_info.viewmat * *tmp_group_ent.transform;
        }
        if (g_State.can_edit && (portal_grp->instance_mods && portal_grp->instance_mods->_TrickFlags & 0x2000))
            continue;
        if (portal_grp->flags & 0x20)
            continue;
        if (!skip_open)
        {
            DefTracker *portal_trackers = doOpen(portal_grp, portal_grp->def, draw_dist, def_and_draw_scale, 0);
            addToVisibleTraysAndDraw(portal_grp, portal_grp->def, sub_transform, portal_local_mid, draw_dist);
            recurseSubs(portal_grp,portal_trackers,portal_grp->def,draw_dist,sub_transform,portal_mid_nonsq_length,0,ctx);
        }
        recursePortals(portal_grp, portal_grp->def, draw_dist, draw, 0,ctx);
    }
    ctx.depth--;
}
void groupDrawDefTracker(GroupDef *def, DefTracker *tracker, Matrix4x3 *transform_mat, EntLight *light, FxMiniTracker *fx_tracker, float view_scale)
{
    DrawParams draw;
    GroupEnt ent;
    DefDrawContext def_ctx;

    draw.pGrp = nullptr;
    draw.scale = g_State.view.vis_scale * view_scale;
    draw.tint_color_ptr = nullptr;
    draw.tex_binds[0] = nullptr;
    draw.tex_binds[1] = nullptr;
    draw.node_mid = { 0,0,0 };
    ent.m_def = nullptr;
    ent.flags_cache = 0;
    ent.vis_dist_sqr_cache = 0.0;
    ent.vist_dist_cache = 0.0;
    ent.mid_cache = { 0,0,0 };
    ent.transform = nullptr;
    g_fx_light = light;
    g_fx_mini_tracker = fx_tracker;
    makeAGroupEnt(ent, def, &Unity_Matrix, draw.scale);
    segs_drawDefInternal(ent, *transform_mat, tracker, 3, &draw, false, def_ctx);
    g_fx_light = nullptr;
    g_fx_mini_tracker = nullptr;
}
static float skyGetFogFarDist()
{
    if ( parsed_scene.ClipFoggedGeometry )
        return g_FogStartEnd.startEnd.y;
    return 0.0f;
}
int segs_groupDrawRefs(const Matrix4x3 &parent_mat)
{
    DrawParams draw;
    int vis = 0;
    initSwayTable();
    sway_count += g_TIMESTEP;
    if (sway_count > 1000000.0f)
        sway_count = 0.0;
    draw.pGrp = nullptr;
    draw.tint_color_ptr = nullptr;
    draw.tex_binds[0] = nullptr;
    draw.tex_binds[1] = nullptr;
    draw.node_mid = {0,0,0};
    draw.scale = g_State.view.vis_scale;
    checkForVisScaleChange(g_State.view.vis_scale);
    fog_far_dist = skyGetFogFarDist();
    if (inEditMode())
        fog_far_dist = 0.0;
    g_curr_fog_dist = 1.6e17f;
    if (inEditMode() || g_State.see_everything)
    {
        g_see_everything = 1;
        if (!g_edit_state.showvis)
            vis = 3;
    }
    else
    {
        g_see_everything = 0;
    }
    see_outside = 1;
    camera_is_inside = 0;
    ++draw_id;
    if (vis != 3)
    {
        DefTracker *trkr = groupFindInside(&cam_info.cammat.TranslationPart, 2); //FINDINSIDE_TRAYONLY
        if (trkr) {
            DefDrawContext def_ctx;
            GroupEnt       ent;
            camera_is_inside      = 1;
            see_outside           = 0;
            makeAGroupEnt(ent, trkr->def, &trkr->matrix1, draw.scale);
            segs_drawDefInternal(ent, parent_mat, trkr, 1, &draw, false,def_ctx);
        }
    }
    if (see_outside || vis==3)
    {
        for (int i = 0; i < group_info.ref_count; ++i)
        {
            DefTracker *tracker = group_info.refs[i];
            if (!tracker->def || tracker->flags & 0x10)
                continue;
            DefDrawContext def_ctx;
            def_ctx.ref_idx = i;
            GroupEnt ent;
            makeAGroupEnt(ent, tracker->def, &tracker->matrix1, draw.scale);
            segs_drawDefInternal(ent, parent_mat, tracker, vis, &draw, false, def_ctx);
        }
    }
    return selDraw();
}
#pragma pack(push, 1)
struct Parser_GroupLoc
{
    char *name;
    Vector3 pos;
    Vector3 rot;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Parser_Def
{
    char *name;
    char *p_Obj;
    char *type;
    Parser_GroupLoc **p_Grp;
    struct Parser_GroupProperty **p_Property;
    struct Parser_TintColor **p_TintColor;
    struct Parser_Sound **p_Sound;
    struct Parser_ReplaceTex **p_ReplaceTex;
    struct Parser_Omni **p_Omni;
    struct Parser_Beacon **p_Beacon;
    struct Parser_Fog **p_Fog;
    struct Parser_Ambient **p_Ambient;
    struct Parser_Lod **p_Lod;
    int flags;
};
#pragma pack(pop)
struct NameList;
extern "C"
{
    __declspec(dllimport) void groupRename(NameList *, const char *, char *, int );
    __declspec(dllimport) int error_ReportParsing(const char *filename, const char *fmt, ...);
    __declspec(dllimport) GroupDef *findNameInLib_P(const char *);
    __declspec(dllimport) void *getMemFromPool(MemPool *pool, const char *filename, int line);
    __declspec(dllimport) int fn_4C13C0(const char *);
    __declspec(dllimport) void Rotation_ZXY_Order(Matrix3x3 *mat, Vector3 *angles);
}
void segs_addGroups(Parser_Def *parser_def, GroupDef *pDef, NameList *name_list, const char *filename) //groupfileload_4C1DA0
{
    char buf[800]={0};
    int  cnt       = COH_ARRAY_SIZE(parser_def->p_Grp);
    pDef->num_subs = cnt;
    if (!pDef->num_subs)
        return;

    pDef->subdefs = (GroupEnt *)COH_CALLOC(sizeof(GroupEnt), pDef->num_subs);
    int valid_dev_idx = 0;
    for(int idx = 0; idx < cnt; ++idx)
    {
        groupRename(name_list, parser_def->p_Grp[idx]->name, buf, 0);
        pDef->subdefs[valid_dev_idx].m_def = findNameInLib_P(buf);
        if (!pDef->subdefs[valid_dev_idx].m_def)
        {
            fn_4C13C0(buf);
            pDef->subdefs[valid_dev_idx].m_def = findNameInLib_P(buf);
        }
        pDef->subdefs[valid_dev_idx].transform = (Matrix4x3 *)getMemFromPool(group_info.matrix_pool_PP, __FILE__, __LINE__);
        Matrix4x3 *transform = const_cast<Matrix4x3 *>(pDef->subdefs[valid_dev_idx].transform);
        assert(&Unity_Matrix!=transform);
        transform->TranslationPart           = parser_def->p_Grp[idx]->pos;
        Rotation_ZXY_Order(&transform->ref3(), &parser_def->p_Grp[idx]->rot);
        //assert(std::abs(transform->r1[2]) != 1.0);
        if (pDef->subdefs[valid_dev_idx].m_def)
        {
            ++valid_dev_idx;
        }
        else
        {
            error_ReportParsing(filename, "Group %s\ncan't find member %s\n", pDef->ref_name,
                                parser_def->p_Grp[idx]->name);
            --pDef->num_subs;
        }
    }
}

void patch_groupdraw()
{
    patchit("groupfileload_4C1DA0", (void *)segs_addGroups);
    patchit("fn_4DBA60", (void *)shadowVolumeVisible);
    patchit("fn_492140", (void *)groupDrawDefTracker);
    patchit("seqgraphics_4979B0",(void *)seqSetStaticLight);
    BREAK_FUNC(drawDefInternal);
    BREAK_FUNC(groupDrawRefs);
    BREAK_FUNC(seqgraphics_getBoneNodes);
}
