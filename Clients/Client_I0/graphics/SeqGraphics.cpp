#include "SeqGraphics.h"

#include "gfx.h"

#include "entity/EntClient.h"
#include "renderer/RenderTricks.h"
#include "renderer/RendererState.h"
#include "renderer/RendererUtils.h"
#include "utils/dll_patcher.h"
#include "GameState.h"
#include "GL/glew.h"

#include <thread>
#include <string.h>
#include "renderer/RenderParticles.h"
struct ParticleSys1;
struct Entity;
extern "C"
{
__declspec(dllimport) void get_screen_size(int *,int *);
__declspec(dllimport) int fn_4F4A60(SeqInstance *, float timestep, int move_idx, char move_updated); //seqProcessClientInst
__declspec(dllimport) void seqgraphics_496670(SeqInstance *, float );//seqSetLod
__declspec(dllimport) GfxTree_Node *gfx_tree_root;
__declspec(dllimport) Entity *getControlledPlayerEntity();
__declspec(dllimport) int boneNameToIdx(const char *);
__declspec(dllimport) GfxTree_Node *gfxTreeFindBoneInAnimation(int boneidx, GfxTree_Node *start, Handle seqhandle);
__declspec(dllimport) void gfxTreeFindWorldSpaceMat(Matrix4x3 *src, GfxTree_Node *);
__declspec(dllimport) void toggle_3d_game_modes(int);
__declspec(dllimport) int particle_4ACB90(ParticleSys1 *, int , Matrix4x3 *);
__declspec(dllimport) void lightEnt(EntLight *light, Vector3 *pos, float min_ambient, float max_ambient);
__declspec(dllimport) void set_func_D4AC20(void (*fn)(struct CamPos *)); // set input->camera state mapping function
__declspec(dllimport) void animSetHeader(SeqInstance *seq);
__declspec(dllimport) void animPlayInst(SeqInstance *seq);
__declspec(dllimport) void fn_5B6740(float yaw, Matrix3x3 *rot_mat);//yawMat3
__declspec(dllimport) void waitForAllAnimsToLoad();
__declspec(dllimport) void waitForAllTexturesToLoad();

__declspec(dllimport) CameraInfo cam_info;
}
static void cameraLocked(CamPos *)
{
    ; // does nothing->camera is locked.
}
uint8_t *takeAPicture(SeqInstance *seq, Vector3 *offset, float fovy, GLsizei width, GLsizei height,
                   const char *focus_on_bone, int half_resolution, int flip, int bodyshot)
{
    int y_offsets[2];
    uint8_t *data;
    unsigned int pixel_accumulator[4];
    uint32_t *original_pixels;
    unsigned int *tgt_mem;
    int width_w;
    int y;
    int x;
    int height_w;
    Entity *player;
    Matrix4x3 particleMat;
    int boneidx;
    int particleId1;
    CameraInfo saved_caminfo;
    int particleId2;
    int disable2d;
    ParticleSys1 *psys2;
    int fov_custom;
    bool hidePlayer;
    int game_mode;
    Matrix4x3 headMat;
    ParticleSys1 *psys1;
    Matrix4x3 dest;
    uint8_t *pixels;

    dest= seq->gfx_root->mat;
    seq->gfx_root->mat = Unity_Matrix;
    particleId1 = 0;
    psys1 = nullptr;
    particleId2 = 0;
    psys2 = nullptr;
    int prev_val = g_State.view.unkn_518;
    g_State.view.unkn_518 = 1;
    fn_4F4A60(seq, 1.0, 0, 1);
    seqgraphics_496670(seq, 1.0);
    seq->seqGfxData.alpha = 255;
    animSetHeader(seq);
    animPlayInst(seq);
    if ( focus_on_bone )
    {
        boneidx = boneNameToIdx(focus_on_bone);
        GfxTree_Node *focus_node = gfxTreeFindBoneInAnimation(boneidx, seq->gfx_root->children_list, seq->seq_instance_handle);
        gfxTreeFindWorldSpaceMat(&headMat, focus_node);
    }
    else
    {
        headMat = seq->gfx_root->mat;
    }
    disable2d = g_State.gDisable2D;
    g_State.gDisable2D = 1;
    fov_custom = int32_t(g_State.view.fov_custom);
    game_mode = g_State.view.game_mode;
    saved_caminfo = cam_info;
    toggle_3d_game_modes(2);
    seq->gfx_root->flg &= ~0x20000;
    cam_info.mat.ref3() = Unity_Matrix.ref3();
    cam_info.mat.TranslationPart = headMat.TranslationPart;
    cam_info.cammat.ref3() = Unity_Matrix.ref3();
    cam_info.cammat.TranslationPart = headMat.TranslationPart + *offset;
    set_func_D4AC20(cameraLocked);
    gfxWindowReshapeForHeadShot(fovy);
    particleMat =cam_info.mat;
    particleMat.TranslationPart.z -= 10.0f;
    if ( bodyshot )
    {
        psys1 = segs_partCreateSystem("hardcoded/bodyshot/bodyshot.part", &particleId1, 0, 10);
        psys2 = segs_partCreateSystem("hardcoded/bodyshot/bodyshotBk.part", &particleId2, 0, 10);
    }
    else
    {
        psys1 = segs_partCreateSystem("hardcoded/headshot/headshot.part", &particleId1, 0, 10);
        psys2 = segs_partCreateSystem("hardcoded/headshot/headshotBk.part", &particleId2, 0, 10);
    }
    particle_4ACB90(psys1, particleId1, &particleMat); //partSetOriginMat
    particle_4ACB90(psys2, particleId2, &particleMat);
    lightEnt(&seq->seqGfxData.light, &seq->gfx_root->mat.TranslationPart, seq->ent_type_info.MinimumAmbient, 0.2f);
    fn_4F4A60(seq, 1.0, 0, 1); //seqProcessClientInst
    seqgraphics_496670(seq, 1.0); //seqSetLod
    seq->seqGfxData.alpha = 255;
    animSetHeader(seq);
    animPlayInst(seq);

    //TODO:
//    glEnable(GL_DEPTH_TEST);
//    segs_wcw_statemgmt_setDepthMask(1u);
//    glDepthMask(1u);
    for (auto iter = gfx_tree_root; iter; iter = iter->next )
    {
        if ( iter->children_list )
        {
            if ( iter->children_list->seqHandle != seq->seq_instance_handle )
                iter->flg |= 0x20000;
        }
    }
    player = getControlledPlayerEntity();
    hidePlayer = false;
    if ( player && player->seq && player->seq->gfx_root )
    {
        hidePlayer = (player->seq->gfx_root->flg & 0x20000) != 0;
        player->seq->gfx_root->flg |= 0x20000;
    }
    fn_5B6740(-0.2f, &seq->gfx_root->mat.ref3());
    segs_sunGlareDisable();
    segs_gfxUpdateFrame(false, true);
    waitForAllAnimsToLoad();
    waitForAllTexturesToLoad();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    segs_gfxUpdateFrame(false, true);
    segs_gfxUpdateFrame(false, true);
    segs_gfxUpdateFrame(false, true);
    if ( !hidePlayer )
    {
        Entity *e = getControlledPlayerEntity();
        if ( e )
        {
            if ( e->seq && e->seq->gfx_root )
                e->seq->gfx_root->flg &= ~0x20000;
        }
    }
    toggle_3d_game_modes(game_mode);
    g_State.view.game_mode = game_mode;
    g_State.gDisable2D = disable2d;
    cam_info = saved_caminfo;
    g_State.view.fov_custom = fov_custom;
    segs_releaseParticleSystem(psys1, particleId1);
    segs_releaseParticleSystem(psys2, particleId2);
    g_State.view.unkn_518 = prev_val;
    get_screen_size(&width_w, &height_w);
    x = int32_t(width_w / 2.0f - width / 2.0f);
    y = int32_t(height_w / 2.0f - height / 2.0f);
    pixels = (uint8_t *)COH_MALLOC(4 * height * width);
    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    if ( flip )
    {
        uint8_t *flipped = (uint8_t *)COH_MALLOC(4 * height * width);
        for (int j = 0; j < height; ++j )
            memcpy(flipped + 4 * width * (height - (j + 1)), &pixels[j * 4 * width], 4 * width);
        COH_FREE(pixels);
        pixels = flipped;
    }
    if ( half_resolution )
    {
        original_pixels = (uint32_t *)pixels;
        data = (uint8_t *)COH_MALLOC(4 * (height /2) * (width / 2));
        tgt_mem = (uint32_t *)data;
        for (int k = 0; k < height/2; ++k )
        {
            y_offsets[0] = width * 2 * k;
            y_offsets[1] = width * (2 * k + 1);
            for (int l = 0; l < width >> 1; ++l )
            {
                for (int m = 0; m < 4; ++m )
                    pixel_accumulator[m] = 0;
                for (int m = 0; m < 4; ++m )
                {
                    uint32_t rgba = original_pixels[(m & 1) + y_offsets[m >> 1] + 2 * l];
                    pixel_accumulator[0] += rgba & 0xFF;
                    pixel_accumulator[1] += (rgba >>  8) & 0xFF;
                    pixel_accumulator[2] += (rgba >> 16) & 0xFF;
                    pixel_accumulator[3] += (rgba >> 24) & 0xFF;
                }
                *tgt_mem = ((pixel_accumulator[3] / 4) << 24) | ((pixel_accumulator[2]/4) << 16) | ((pixel_accumulator[1] / 4) << 8) | (pixel_accumulator[0] / 4);
                ++tgt_mem;
            }
        }
        COH_FREE(pixels);
        pixels = data;
    }
    seq->gfx_root->mat = dest;
    segs_gfxWindowReshape();
    return pixels;
}
void patch_seqgraphics()
{
    patchit("seqgraphics_497D10",reinterpret_cast<void *>(takeAPicture));
}
