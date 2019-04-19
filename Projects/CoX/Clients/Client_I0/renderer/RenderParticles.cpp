#include  "RenderParticles.h"

#include "utils/dll_patcher.h"
#include "GameState.h"
#include "Texture.h"
#include "RenderBonedModel.h"
#include "Model.h"
#include "RendererState.h"
#include "ShaderProgramCache.h"
#include "RenderTricks.h"
#include "RendererUtils.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <algorithm>

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif
#pragma pack(push,1)
struct RGB
{
    uint8_t r, g, b;
};
#pragma pack(pop)
struct ParticleQuad
{
    Vector2 v[4];
};
struct ParticleQuadIdx
{
    Vector3i v[2];
};
struct ColorFx
{
    RGBA startcolor;
    int startTime;
};
struct PreparedColorFx
{
    struct RGB *entries;
    int length;
};

struct ParticleTexture
{
    char m_TextureName[64];
    Vector3 m_TexScroll;
    Vector3 m_TexScrollJitter;
    float m_AnimFrames;
    float m_AnimPace;
    uint32_t m_AnimType;
    Vector2 m_texScale;
    int framewidth;
    TextureBind *bind;
};
enum eExpandTypes {
    // Expansion is based on timestep * m_ExpandRate
    Expand_Unbound = 0, // Unbound expansion
    Expand_Bound = 1, // grow until size reaches m_EndSize
    Expand_PingPong = 2, // grow from m_StartSize to m_EndSize, and than back to m_StartSize
};
struct Parse_PSystem
{
    Parse_PSystem *next;
    Parse_PSystem *prev;
    int initialized;
    int last_file_change;
    uint32_t m_WorldOrLocalPosition;
    uint32_t m_FrontOrLocalFacing;
    float m_TimeToFull;
    uint32_t m_KickStart;
    float *m_NewPerFrame;
    int *m_Burst;
    float m_MoveScale;
    uint32_t m_BurbleType;
    float m_BurbleFrequency;
    float *m_BurbleAmplitude;
    float m_BurbleThreshold;
    uint32_t m_EmissionType;
    Vector3 *m_EmissionStartJitter;
    float m_EmissionRadius;
    float m_EmissionHeight;
    int tex_spin;
    int tex_spin_jitter;
    uint32_t m_Spin;
    uint32_t m_SpinJitter;
    uint32_t m_OrientationJitter;
    float m_Magnetism;
    float m_Gravity;
    uint32_t m_KillOnZero;
    uint32_t m_Terrain;
    Vector3 *m_InitialVelocity;
    Vector3 *m_InitialVelocityJitter;
    Vector3 m_VelocityJitter;
    float m_TightenUp;
    float m_SortBias;
    float m_Drag;
    float m_Stickiness;
    int *m_Alpha;
    uint32_t m_ColorChangeType;
    ColorFx m_StartColor[5];
    PreparedColorFx colorPath;
    float m_FadeInBy;
    float m_FadeOutStart;
    float m_FadeOutBy;
    uint32_t m_Blend_mode;
    float *m_StartSize;
    float m_StartSizeJitter;
    float *m_EndSize;
    float m_ExpandRate;
    uint32_t m_ExpandType;
    uint32_t m_StreakType;
    uint32_t m_StreakOrient;
    uint32_t m_StreakDirection;
    float m_StreakScale;
    ParticleTexture parttex[2];
    char *m_Name;
    char m_DieLikeThis[128];
    uint32_t m_DeathAgeToZero;
    uint32_t m_Flags;
    float m_VisRadius;
    float m_VisDist;
    int curralpha;
};
static_assert(sizeof(Parse_PSystem) == 0x280);
struct ParserRelatedStruct;
struct ListEntry
{
    ListEntry *next;
    ListEntry *prev;
};
#pragma pack(push, 1)
struct ParserRel_Sub1
{
    XFileHandle *fh;
    int offset;
    ParserRel_Sub1 *parent_sourcefile;
    int line_no;
    int line;
    char filename[260];
};
#pragma pack(pop)

struct ParserRelatedStruct
{
    char buffer[24000];
    int counter;
    int buffer_size;
    unsigned int flag;
    ParserRel_Sub1 *current_sourcefile;
    struct SymbolTable *sym1;
    struct SymbolTable *sym2;
    CharAttr_Nested1 *token_defs;
};

extern "C" {
__declspec(dllimport) void utils_report_log_to_file1(const char *fmt, ...);
__declspec(dllimport) int ParserLoadFiles(const char *, const char *alternate_ext, const char *, int flags, CharAttr_Nested1 *loading_template, void *tgt_memory, void *, int *, signed int(*)(void *, void *));
__declspec(dllimport) int fileLastChanged(const char *lpFileName);
__declspec(dllimport) int fileHasBeenUpdated(char *lpFileName, int *d);
__declspec(dllimport) void free_and_remove_from_list(ListEntry *, ListEntry **);
__declspec(dllimport) ListEntry *alloc_and_add_to_list(ListEntry *tail, size_t size, const char *filename, int linenumber);
__declspec(dllimport) ParserRelatedStruct *CreateTokenizer(const char *file_to_parse);
__declspec(dllimport) int fn_5BE550(ParserRelatedStruct *str_to_parse, CharAttr_Nested1 *valid_tokens, char *dat, int (*tok_func)(ParserRelatedStruct *, const char *, char *));
__declspec(dllimport) int PrintfConsole1(const char *, ...);
__declspec(dllimport) void font_489280(int l, const char *fmt, ...);
__declspec(dllimport) void Destroy_XFileHandle(XFileHandle **);
__declspec(dllimport) void ErrorfInternal(const char *fmt, ...);
__declspec(dllimport) void particle_4ACDE0(PSystem_1AC *);
__declspec(dllimport) void ErrorfFL(const char *filename, int line);
__declspec(dllimport) void xyprintf(int x, int y, const char *fmt, ...);
__declspec(dllimport) int  IsDevelopmentMode();
__declspec(dllimport) void font__489250();
__declspec(dllimport) void pitchMat3(float pitch_angle, Matrix3x3 *tgt);
__declspec(dllimport) ParticleSys1 *fn_4ABBF0(ParticleSys1 *first, int , int(*cmp_fnc)(ParticleSys1 *, ParticleSys1 *, ParticleSys1 *), int, int);
__declspec(dllimport) void particle_4ACEF0(ParticleSys1 *);
__declspec(dllimport) void ReverseTransform(Matrix4x3 *,Matrix4x3 *);
__declspec(dllimport) void partInitSystemInfo(Parse_PSystem *);
__declspec(dllimport) int UnrecognizedToken(ParserRelatedStruct *ctx, const char *token, char *following_text);
__declspec(dllimport) CameraInfo cam_info;
__declspec(dllimport) TextureBind *g_whiteTexture;
__declspec(dllimport) float g_TIMESTEP;
__declspec(dllimport) CharAttr_Nested1 ParticleSystem_Token[2];
__declspec(dllimport) CharAttr_Nested1 ParticleTokens[86];
}
static ParticleGlobal g_particle_engine;
static float g_precomp_sin[513];
static float g_precomp_cos[513];
static Parse_PSystem **dword_7F8898;

static int segs_rdrPrepareToRenderParticleSystems(MaterialDefinition &part_mat)
{
    part_mat.draw_data.setDiscardAlpha(0.0);
    part_mat.set_useAlphaDiscard(1);
    part_mat.render_state.setCullMode(RenderState::CULL_NONE);
    part_mat.render_state.setDepthWrite(false);
    part_mat.set_shadeModel(MaterialDefinition::FLAT);
    g_particle_engine.smallSystemsIdx = 0;
    g_particle_engine.mediumSystemsIdx = 0;
    g_particle_engine.largeSystemsIdx = 0;
    part_mat.draw_data.modelViewMatrix = glm::mat4(1);
    segs_texSetAllToWhite();
    segs_wcw_UnBindBufferARB();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_particle_engine.indices_buf_id);
    glBindBuffer(GL_ARRAY_BUFFER, g_particle_engine.uv_coords_buf);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return 0;
}
namespace {
    // find vec-vec angle index, the index is used to lookup values in g_precomp_* arrays
    int findAngle(Vector3 *vec1, Vector3 *vec2)
    {
        float calcd;
        Vector3 diff = *vec2 - *vec1;
        int sign = 1;
        if (diff.x == 0.0f)
            return 64.0;
        float diff_y_over_x = diff.y / diff.x;
        float abs_y_over_x = std::abs(diff_y_over_x);
        if (diff_y_over_x < 0.0f)
        {
            sign = -1;
        }
        if (abs_y_over_x <= 1.0f)
            calcd = abs_y_over_x / (abs_y_over_x * abs_y_over_x * 0.280872f + 1.0f);
        else
            calcd = glm::half_pi<float>() - abs_y_over_x / (abs_y_over_x * abs_y_over_x + 0.280872f);
        float res = sign * calcd * 40.74368f;
        if (diff.x < 0.0f)
            res += 128.0f;
        return int(res);
    }

    void particle_partBuildParticleArray(ParticleSys1 *sys, float alpha, float *verts, char *rgb_memory, float step)
    {
        char angle_idx;
        int color_path_idx;
        Vector3 facing_pos_1;
        Vector3 facing_pos_2;

        Vector3 emission_start = sys->emission_start;
        Parse_PSystem *sys_info = sys->sysInfo;
        int part_tgt_idx = sys->maxParticles;
        float *tgt_vertices = verts;
        for (int i = 0; i < sys->particle_count; ++i, ++part_tgt_idx)
        {
            if (part_tgt_idx == sys->maxVertParticles)
                part_tgt_idx = 0;

            PSystem_1AC *part_entry = &sys->particles[part_tgt_idx];
            float part_size = part_entry->size;
            facing_pos_1    = sys->facingMat * part_entry->m_pos;
            facing_pos_1.z += sys_info->m_TightenUp;

            if (sys_info->m_StreakType)
            {
                Vector3 pos_vec = part_entry->m_pos;
                switch (sys_info->m_StreakType)
                {
                case 1:
                {
                    Vector3 mod_vec = part_entry->m_velocity * sys_info->m_StreakScale;
                    if (sys_info->m_StreakDirection)
                        pos_vec += mod_vec;
                    else
                        pos_vec -= mod_vec;
                    break;
                }
                case 2:
                {
                    pos_vec.y += sys_info->m_StreakScale * sys_info->m_Gravity;
                    break;
                }
                case 3:
                    pos_vec = emission_start;
                    break;
                case 4:
                    pos_vec = sys->magnet_vector;
                    break;
                case 5:
                {
                    pos_vec = sys->other_vec;
                    break;
                }
                case 6:
                    break;
                case 7:
                    if (i < sys->particle_count - 1)
                    {
                        int next_part_idx = part_tgt_idx + 1;
                        if (next_part_idx == sys->maxVertParticles)
                            next_part_idx = 0;
                        pos_vec = sys->particles[next_part_idx].m_pos;
                    }
                    else
                    {
                        pos_vec = emission_start;
                    }
                    break;
                }
                facing_pos_2 = sys->facingMat * pos_vec;
                facing_pos_2.z += sys_info->m_TightenUp;
            }
            else
                facing_pos_2 = facing_pos_1;
            if (sys_info->m_StreakType && sys_info->m_StreakOrient == 1)
            {
                if (sys_info->m_StreakDirection == 1)
                {
                    std::swap(facing_pos_1, facing_pos_2);
                }
                angle_idx = char(findAngle(&facing_pos_2, &facing_pos_1) - 64);
            }
            else
            {
                part_entry->theta += char(part_entry->spin);
                angle_idx = char(part_entry->theta);
            }
            // fill VBO with a quad
            tgt_vertices[0] = part_size * g_precomp_cos[uint8_t(angle_idx + 96)] + facing_pos_1.x;
            tgt_vertices[1]  = part_size * g_precomp_sin[uint8_t(angle_idx + 96)] + facing_pos_1.y;
            tgt_vertices[2] = facing_pos_1.z;
            tgt_vertices[3]  = part_size * g_precomp_cos[uint8_t(angle_idx + 32)] + facing_pos_1.x;
            tgt_vertices[4]  = part_size * g_precomp_sin[uint8_t(angle_idx + 32)] + facing_pos_1.y;
            tgt_vertices[5] = facing_pos_1.z;
            tgt_vertices[6]  = part_size * g_precomp_cos[uint8_t(angle_idx - 96)] + facing_pos_2.x;
            tgt_vertices[7]  = part_size * g_precomp_sin[uint8_t(angle_idx - 96)] + facing_pos_2.y;
            tgt_vertices[8] = facing_pos_2.z;
            tgt_vertices[9] = part_size * g_precomp_cos[uint8_t(angle_idx - 32)] + facing_pos_2.x;
            tgt_vertices[10] = part_size * g_precomp_sin[uint8_t(angle_idx - 32)] + facing_pos_2.y;
            tgt_vertices[11] = facing_pos_2.z;
            tgt_vertices += 12;
        }
        int used_part_idx   = sys->maxParticles;
        char *tgt_rgb       = rgb_memory;
        RGB * colors           = (RGB *)sys_info->m_StartColor;
        for (int part = 0; part < sys->particle_count; ++part)
        {
            if (used_part_idx == sys->maxVertParticles)
                used_part_idx = 0;
            PSystem_1AC *part_entry = &sys->particles[used_part_idx];
            if (sys_info->colorPath.length > 1)
            {
                float part_age = part_entry->age - step;
                if (sys_info->m_ColorChangeType) // cyclic
                {
                    color_path_idx = int(part_age) % sys_info->colorPath.length;
                }
                else // upto age
                {
                    color_path_idx = std::min<int>(part_age,sys_info->colorPath.length - 1);
                }
                colors = &sys_info->colorPath.entries[color_path_idx];
            }
            uint8_t comp_alpha = uint8_t(part_entry->alpha * alpha);
            for (int i = 0; i<4; ++i)
            {
                tgt_rgb[0] = colors->r;
                tgt_rgb[1] = colors->g;
                tgt_rgb[2] = colors->b;
                tgt_rgb[3] = comp_alpha;
                tgt_rgb += 4;
            }
            ++used_part_idx;
        }
    }
int segs_modelDrawParticleSys(ParticleSys1 *system, float alpha, GLuint buffer, Matrix4x3 *systemMatCamSpace,MaterialDefinition &material) //renderparticles_4D9790
{
    void *rgba_arr;
    void *verts;

    Parse_PSystem *sys_info = system->sysInfo;
    if (systemMatCamSpace)
    {
        material.draw_data.modelViewMatrix = systemMatCamSpace->toGLM();
    }
    else
    {
        material.draw_data.modelViewMatrix = glm::mat4(1);
    }
    assert(buffer);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    rgba_arr = (char *)system->rgbas + (unsigned int)verts;
    assert(verts);
    if (!(g_State.view.fx_debug_req & 0x80))
        particle_partBuildParticleArray(system, alpha, (float *)verts, (char *)rgba_arr, g_TIMESTEP);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    static GeometryData fakevbo;
    if (!fakevbo.segs_data)
        fakevbo.createVAO();
    fakevbo.setVertexBuffer(buffer);
    fakevbo.gl_index_buffer = g_particle_engine.indices_buf_id;
    fakevbo.setColorBuffer({ fakevbo.gl_vertex_buffer , uint32_t(system->rgbas) });
    fakevbo.setUVVbo(g_particle_engine.uv_coords_buf);

    if (sys_info->m_Blend_mode == 1)
    {
        material.render_state.setBlendMode(RenderState::BLEND_ADDALPHA);
        material.draw_data.fog_params.enabled = false;
    }
    else
    {
        material.render_state.setBlendMode(RenderState::BLEND_ALPHA);
        material.draw_data.fog_params.enabled = true;
    }
    TextureBind *tex0 = g_State.view.whiteParticles ? g_whiteTexture : sys_info->parttex[0].bind;
    TextureBind *tex1 = g_State.view.whiteParticles ? g_whiteTexture : sys_info->parttex[1].bind;
    segs_texBindTexture(GL_TEXTURE_2D, 0, tex0);
    segs_texBindTexture(GL_TEXTURE_2D, 1, tex1);
    material.draw_data.tex_id_0 = tex0;
    glm::mat4x4 tex_mat(1);
    tex_mat = glm::translate(tex_mat, { system->m_perTextureTranslate[0].x, system->m_perTextureTranslate[0].y, 0.0 });
    tex_mat = glm::scale(tex_mat, { sys_info->parttex[0].m_texScale.x, sys_info->parttex[0].m_texScale.y, 0.0 });
    material.draw_data.textureMatrix0 = tex_mat;
    tex_mat = glm::mat4x4(1);
    tex_mat = glm::translate(tex_mat, { system->m_perTextureTranslate[1].x, system->m_perTextureTranslate[1].y, 0.0 });
    tex_mat = glm::scale(tex_mat, { sys_info->parttex[1].m_texScale.x, sys_info->parttex[1].m_texScale.y, 0.0 });
    material.draw_data.textureMatrix1 = tex_mat;
    material.apply();
    if (!(g_State.view.fx_debug_req & 0x100))
        fakevbo.draw(*material.program, GL_TRIANGLES,6 * system->particle_count,0);
    return system->particle_count;
}
}
void segs_rdrCleanUpAfterRenderingParticleSystems()
{
    RenderState reset_state = g_render_state.getGlobal();
    reset_state.setBlendMode(RenderState::BLEND_ALPHA);
    reset_state.setDepthWrite(true);
    reset_state.setCullMode(RenderState::CULL_CCW);
    reset_state.setDepthTestMode(RenderState::CMP_LESSEQUAL);
    g_render_state.apply(reset_state);

    segs_wcw_UnBindBufferARB();
}
static int partCompareSysDist(ParticleSys1 *sysOne, ParticleSys1 *sysTwo, ParticleSys1 * /*unused*/)
{
    const float distOne = sysOne->totalAlpha - sysOne->sysInfo->m_SortBias;
    const float distTwo = sysTwo->totalAlpha - sysTwo->sysInfo->m_SortBias;
    if (distOne > distTwo)
        return -1;
    if (distOne < distTwo)
        return 1;
    return 2 * (sysOne->unique_id < sysTwo->unique_id) - 1;
}
static ParticleSys1 *segs_partSortByDistance(ParticleSys1 *first)
{

    for (ParticleSys1 *i = first; i; i = i->next)
    {
        i->vec_sq_length = (cam_info.cammat.TranslationPart - i->drawMat.TranslationPart).lengthNonSqrt() - i->sysInfo->m_TightenUp;
    }
    ParticleSys1 *sorted_start = fn_4ABBF0(first, 0, partCompareSysDist, 0, 0);
    if (sorted_start)
        sorted_start->prev = nullptr;
    for(ParticleSys1 *iter = sorted_start;iter;iter = iter->next)
    {
        if (iter->next)
            iter->next->prev = iter;
    }
    return sorted_start;
}
int32_t rand3() {
    static uint32_t seed3 = 0xC9442554;
    seed3 = 0x343FD * seed3 + 0x269EC3;
    return static_cast<int32_t>(seed3);
}
#define rand_f() (float(rand3())/float(INT_MAX))
float noise_v(uint32_t x) // range of -1 to 1
{
    x = (x << 13) ^ x;
    return 1.0f - ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / float(1073741824);
}
static float rand_float_P()
{
    uint32_t v = (rand3() & 0x7FFFFF) | 0x3F800000;
    return (*(float *)&v) - 1.0f;
}
static float interpolatedNoise(float val)
{
    float noise_1 = noise_v(uint32_t(val));
    float noise_2 = noise_v(uint32_t(val) + 1);
    float fixup = val - int(val);
    return noise_1 * (1.0f - fixup) + noise_2 * fixup;
}
static void rand_perturb_vec3(Vector3 &vec, const Vector3 &perturb_scale) {
    vec.x += rand_f() * perturb_scale.x;
    vec.y += rand_f() * perturb_scale.y;
    vec.z += rand_f() * perturb_scale.z;
}
static void partGetRandomPointOnLine(Vector3 *pt1, Vector3 *pt2, PSystem_1AC *part)
{

    if (pt1->close_enough_to(*pt2))
    {
        part->m_pos = *pt1;
    }
    else
    {
        Vector3 diff = *pt2 - *pt1;
        part->m_pos.x = rand_float_P() * diff.x;
        part->m_pos.y = diff.y * part->m_pos.x / diff.y;
        part->m_pos.z = diff.z * part->m_pos.x / diff.z;
        part->m_pos += *pt1;
    }
}
static float lookupPrecompCos(int theta)
{
    assert(theta >= 0 && theta < 512);
    return g_precomp_cos[theta];
}
static float lookupPrecompSin(int theta)
{
    assert(theta >= 0 && theta < 512);
    return g_precomp_sin[theta];
}
static void  getRandomPointOnCylinder(Vector3 *center, float radius, float height, PSystem_1AC *part)
{
    int theta = rand3() & 0xFF;
    part->m_pos.x = lookupPrecompCos(theta) * radius + center->x;
    part->m_pos.y = rand_float_P() * height + center->y;
    part->m_pos.z = lookupPrecompSin(theta) * radius + center->z;
}
static int randomSign()
{
    return (rand3() & 1) ? 1 : -1;
}
static void getRandomPointOnSphere(Vector3 *center, float radius, PSystem_1AC *part)
{
    static Vector3 s_vec;
    static Vector3 initial_vec1{ -0.36514801f, 0.182574f, 0.912871f };
    static Vector3 initial_vec2{ 0.371391f, -0.55708599f, 0.74278098f };

    s_vec = initial_vec2;
    initial_vec2 += initial_vec1;
    initial_vec1 = s_vec;
    initial_vec2.normalize();
    //BUG: this should swap random components - COH didn't do it correctly
#ifdef ENABLE_BUGFIX
    std::swap(initial_vec2.x, initial_vec2[rand() % 3]);
    std::swap(initial_vec2.y, initial_vec2[rand() % 3]);
    std::swap(initial_vec2.z, initial_vec2[rand() % 3]);
#endif

    initial_vec2.x *= randomSign();
    initial_vec2.y *= randomSign();
    initial_vec2.z *= randomSign();
    part->m_pos.x = initial_vec2.x * radius + center->x;
    part->m_pos.y = initial_vec2.y * radius + center->y;
    part->m_pos.z = initial_vec2.z * radius + center->z;
}

static bool partUpdateSystem(ParticleSys1 *psys, float step_size, float clampedTimestep)
{
    int num_particles_to_add;
    float fadein_val;
    float new_buffer;
    float end_size;
    Vector3 initial_velocity;
    float magnetism;
    int num_new;
    Vector3 emission_start;
    float drag;
    float start_size;
    float expand_rate;
    Vector3 emission_start_jitter;
    float gravity;

    Parse_PSystem *sysdef = psys->sysInfo;
    int maxParticles = psys->maxVertParticles;

    psys->totalSize = 0;
    psys->totalAlpha = 0;
    if (0==sysdef->m_WorldOrLocalPosition && 0==sysdef->m_FrontOrLocalFacing)
    {
        psys->facingMat      = cam_info.viewmat;
        psys->emission_start = psys->drawMat.TranslationPart;
        initial_velocity     = psys->drawMat.ref3() * psys->initialVelocity;
    }
    else if (sysdef->m_WorldOrLocalPosition == 1 && !sysdef->m_FrontOrLocalFacing)
    {
        psys->facingMat      = cam_info.viewmat * psys->drawMat;
        psys->emission_start = Vector3{ 0,0,0 };
        initial_velocity     = psys->initialVelocity;
    }
    else if (0 == sysdef->m_WorldOrLocalPosition && sysdef->m_FrontOrLocalFacing == 1)
    {
        Matrix4x3 dest = psys->drawMat;
        pitchMat3(glm::half_pi<float>(), &dest.ref3());
        ReverseTransform(&dest, &psys->facingMat);
        psys->emission_start = psys->drawMat.TranslationPart;
        initial_velocity     = psys->drawMat.ref3() * psys->initialVelocity;
    }
    else if (sysdef->m_WorldOrLocalPosition != 1 || sysdef->m_FrontOrLocalFacing != 1)
    {
        psys->facingMat      = cam_info.viewmat;
        psys->emission_start = psys->drawMat.TranslationPart;
        initial_velocity     = psys->initialVelocity;
    }
    else
    {
        psys->facingMat = Unity_Matrix;
        pitchMat3(-glm::half_pi<float>(), &psys->facingMat.ref3());
        psys->emission_start = Vector3{ 0,0,0 };
        initial_velocity     = psys->initialVelocity;
    }
    if (0.0f == psys->age || psys->teleported)
    {
        psys->emission_start_prev = psys->emission_start;
    }
    Vector3 velocity_jitter = psys->m_InitialVelocityJitter;
    Vector3 tmp = psys->emission_start - psys->emission_start_prev;
    start_size = psys->m_StartSize;
    end_size = psys->m_EndSize;
    emission_start_jitter = psys->m_EmissionStartJitter;
    psys->totalAlpha = 0;
    psys->totalSize = 0;
    psys->stickinessVec = tmp * sysdef->m_Stickiness;
    magnetism = step_size * sysdef->m_Magnetism;
    gravity = step_size * sysdef->m_Gravity;
    expand_rate = step_size * sysdef->m_ExpandRate;
    drag = step_size * sysdef->m_Drag;
    emission_start = psys->emission_start;
    psys->boxMax = psys->emission_start;
    psys->boxMin = psys->emission_start;
    if (sysdef->m_MoveScale == 0.0f || psys->age == 0.0f)
    {
        new_buffer = clampedTimestep * psys->m_NewPerFrame;
    }
    else if (psys->dying)
    {
        new_buffer = 0.0;
    }
    else if (sysdef->m_MoveScale > 0.0f)
    {
        new_buffer = tmp.length() * sysdef->m_MoveScale * psys->m_NewPerFrame;
    }
    else
    {
        float base = clampedTimestep * psys->m_NewPerFrame;
        float delta = std::max(-1.0f * base, tmp.length() * sysdef->m_MoveScale);
        new_buffer = delta + base;
    }

    if (psys->age == 0.0f)
        new_buffer += psys->m_Burst;
    if (psys->m_BurbleAmplitude != 0.0f)
    {
        float burble_loc = psys->age / sysdef->m_BurbleFrequency;
        float burble_val;
        if (sysdef->m_BurbleType)
        {
            burble_val = g_precomp_sin[int32_t(burble_loc) & 0xFF];
        }
        else
        {
            burble_val = interpolatedNoise(burble_loc / 64.0f);
        }
        if (burble_val >= sysdef->m_BurbleThreshold)
            new_buffer += (burble_val + 1.0f) * psys->m_BurbleAmplitude * clampedTimestep;
    }
    float to_create = new_buffer + psys->new_buffer;
    num_new = int32_t(to_create);
    psys->new_buffer = (to_create - num_new);

    for (uint8_t idx = 0; idx < 2; ++idx) {
        ParticleTexture &sys_part(sysdef->parttex[idx]);
        if (sys_part.m_AnimFrames > 1.0f)
        {
            psys->framecurr[idx] += step_size * sys_part.m_AnimPace;
            switch (sys_part.m_AnimType) {
            case 0: {
                if (psys->framecurr[idx] >= sys_part.m_AnimFrames)
                    psys->framecurr[idx] -= sys_part.m_AnimFrames;
                break;
            }
            case 1: {
                if (psys->framecurr[idx] >= sys_part.m_AnimFrames)
                    psys->framecurr[idx] = sys_part.m_AnimFrames - 1.0f;
                break;
            }
            case 2: {
                if (psys->framecurr[idx] >= sys_part.m_AnimFrames)
                {
                    psys->framecurr[idx] -= sys_part.m_AnimFrames;
                    psys->animpong = psys->animpong == 0;
                }
                break;
            }
            }
            float t_frame;
            if (psys->animpong)
                t_frame = sys_part.m_AnimFrames - psys->framecurr[idx];
            else
                t_frame = psys->framecurr[idx];
            psys->m_perTextureTranslate[idx].x = (float)(int(t_frame) % sys_part.framewidth) * sys_part.m_texScale.x;
            psys->m_perTextureTranslate[idx].y = (float)(int(t_frame) / sys_part.framewidth) * sys_part.m_texScale.y;
        }
        psys->m_perTextureTranslate[idx].x += sys_part.m_TexScroll.x;
        psys->m_perTextureTranslate[idx].y += sys_part.m_TexScroll.y;
        rand_perturb_vec3(sys_part.m_TexScroll, sys_part.m_TexScrollJitter);
    }
    num_particles_to_add = std::min(num_new, maxParticles - psys->particle_count);
    num_particles_to_add = std::min(num_particles_to_add, g_State.view.maxParticles - g_particle_engine.dat.particle_count);
    num_new = num_particles_to_add;
    psys->particle_count += num_particles_to_add;
    assert(psys->particle_count <= 1500);

    for (int idx = 0; idx<num_particles_to_add; idx++)
    {
        PSystem_1AC *particle = &psys->particles[psys->firstParticle++];
        if (psys->firstParticle == maxParticles)
            psys->firstParticle = 0;
        particle->age = 0;
        particle->size = psys->m_StartSize;
        particle->theta = 0;
        particle->pulse_direction = 1;
        if (sysdef->m_StartSizeJitter != 0.0f)
        {
            particle->size = rand_f() * sysdef->m_StartSizeJitter + psys->m_StartSize;
        }
        particle->spin = sysdef->m_Spin;
        if (sysdef->m_SpinJitter)
        {
            particle->spin += rand3() % (signed int)(sysdef->m_SpinJitter + 1);
            if (!particle->spin)
                particle->spin = (rand3() >> 0x1F) | 1; // 1 or -1
        }
        if (sysdef->m_OrientationJitter)
            particle->theta = rand3() % (sysdef->m_OrientationJitter + 1);
        particle->m_velocity = initial_velocity;
        rand_perturb_vec3(particle->m_velocity, velocity_jitter);
        switch (sysdef->m_EmissionType)
        {
        case 0:
            particle->m_pos = emission_start;
            break;
        case 1:
            partGetRandomPointOnLine(&emission_start, &psys->other_vec, particle);
            break;
        case 2:
            getRandomPointOnCylinder(&emission_start, sysdef->m_EmissionRadius, sysdef->m_EmissionHeight, particle);
            break;
        case 3:
            particle->m_pos = emission_start - tmp * (1.0f - (float)idx / (float)num_new);
            break;
        case 4:
            getRandomPointOnSphere(&emission_start, sysdef->m_EmissionRadius, particle);
            break;
        case 5:
            particle->m_pos = (psys->other_vec - emission_start) / (float(num_new) * idx) + emission_start;
            break;
        default:
            assert(false);
            break;
        }
        rand_perturb_vec3(particle->m_pos, emission_start_jitter);
    }
    if (psys->unkn_1A9)
    {
        psys->unkn_1A9 = 0;
        particle_4ACEF0(psys);
    }
    int part_offset = psys->maxParticles;
    for (int idx = psys->particle_count; idx>0; --idx, ++part_offset)
    {
        if (part_offset == psys->maxVertParticles)
            part_offset = 0;

        PSystem_1AC *particle = &psys->particles[part_offset];
        if (particle->age >= sysdef->m_FadeOutBy)
        {
            if (++psys->maxParticles >= psys->maxVertParticles)
                psys->maxParticles = 0;
            --psys->particle_count;
            particle->age = -1.0f;
            continue;
        }
        if (particle->age < sysdef->m_FadeOutStart)
        {
            if (particle->age >= sysdef->m_FadeInBy)
                fadein_val = 1.0f;
            else
                fadein_val = particle->age / sysdef->m_FadeInBy;
        }
        else
        {
            fadein_val = 1.0f - (particle->age - sysdef->m_FadeOutStart) / (sysdef->m_FadeOutBy - sysdef->m_FadeOutStart);
        }
        particle->alpha = std::max<uint8_t>(int32_t(fadein_val*psys->m_Alpha_124), 1);
        rand_perturb_vec3(particle->m_velocity, sysdef->m_VelocityJitter);

        particle->m_velocity.y -= gravity;
        particle->m_velocity -= particle->m_velocity * drag; // reduce velocity by drag

        particle->m_pos += particle->m_velocity * step_size; // update the position
        if (magnetism != 0.0f)
        {
            int sign_x = (particle->m_pos.x >= psys->magnet_vector.x) ? -1 : 1;
            int sign_y = (particle->m_pos.y >= psys->magnet_vector.y) ? -1 : 1;
            int sign_z = (particle->m_pos.z >= psys->magnet_vector.z) ? -1 : 1;
            // increase velocity towards the center of magnetism.
            particle->m_velocity.x += sign_x * magnetism;
            particle->m_velocity.y += sign_y * magnetism;
            particle->m_velocity.z += sign_z * magnetism;
        }
        if (sysdef->m_Terrain == 1)
        {
            particle_4ACDE0(particle); //adjustForTerrain
        }
        if (sysdef->m_Stickiness != 0.0f && particle->age != 0.0f)
        {
            particle->m_pos += psys->stickinessVec;
        }
        if (sysdef->m_ExpandRate != 0.0f) {
            switch (sysdef->m_ExpandType) {
            case Expand_Unbound: {
                particle->size += expand_rate;
                break;
            }
            case Expand_Bound: {
                particle->size += expand_rate;
                if (sysdef->m_ExpandRate < 0.0f)
                    particle->size = std::max(end_size, particle->size);
                else if (sysdef->m_ExpandRate > 0.0f)
                    particle->size = std::min(end_size, particle->size);
                break;
            }
            case Expand_PingPong: {
                float prev_size = particle->size;
                float new_size  = particle->pulse_direction * expand_rate + particle->size;
                particle->size  = new_size;
                if (((prev_size < end_size) && new_size > end_size) ||
                        ((prev_size > end_size) && new_size < end_size) ||
                        ((prev_size < start_size) && new_size > start_size) ||
                        ((prev_size > start_size) && new_size < start_size))
                {
                    particle->size            = prev_size;
                    particle->pulse_direction = -particle->pulse_direction;
                }
                break;
            }
            }
        }
        if (!psys->currDrawState) {
            psys->boxMin          = psys->boxMin.min_val(particle->m_pos);
            psys->boxMax          = psys->boxMax.max_val(particle->m_pos);
            psys->biggestParticle = std::max(psys->biggestParticle, std::abs(particle->size));
            psys->totalSize += particle->size * particle->size;
            psys->totalAlpha += particle->alpha;
        }
        particle->age += step_size;
    }
    switch (sysdef->m_StreakType) {
    case 4:
        psys->boxMin = psys->boxMin.min_val(psys->magnet_vector);
        psys->boxMax = psys->boxMax.max_val(psys->magnet_vector);
        break;
    case 5:
        psys->boxMin = psys->boxMin.min_val(psys->other_vec);
        psys->boxMax = psys->boxMax.max_val(psys->other_vec);
        break;
    default:
        ;
    }
    psys->age += step_size;
    if (!psys->dying && (psys->m_BurbleAmplitude != 0.0f || sysdef->m_MoveScale != 0.0f))
        return true;
    return psys->particle_count || !psys->KillOnZero || new_buffer > 0.0f;
}

static void partKickStartSystem(ParticleSys1 *system)
{
    int drawstate = system->currDrawState;
    while (system->age < system->sysInfo->m_TimeToFull)
        partUpdateSystem(system, 1.0, 1.0);
    system->currDrawState = drawstate;
    system->KickStart = 0;
}
static bool systemIsStillAlive(ParticleSys1 *sys, float dT)
{
    sys->timeSinceLastUpdate += dT;
    // we are not dying and still able to create more particles
    if (!sys->dying && (sys->m_NewPerFrame != 0.0f || sys->sysInfo->m_MoveScale != 0.0f || sys->m_BurbleAmplitude != 0.0f))
        return true;
    // we have particles and not faded out
    if (sys->particle_count && sys->timeSinceLastUpdate < sys->sysInfo->m_FadeOutBy)
        return true;
    return false;
}
static float partCalculatePerformanceAlphaFade(ParticleSys1 *sys)
{
    Vector3 cam_space_mid;
    Parse_PSystem *info = sys->sysInfo;
    Vector3 boxDiagonal = sys->boxMax - sys->boxMin;
    Vector3 mid = boxDiagonal * 0.5f;
    float radius = mid.length() + sys->biggestParticle;

    mid += sys->boxMin;
    if (info->m_WorldOrLocalPosition == 1)
    {
        cam_space_mid = sys->drawMat * mid;
    }
    else
    {
        cam_space_mid = mid;
    }
    mid = cam_info.viewmat * cam_space_mid;
    if (gfxSphereVisible(&mid, radius) == 0)
        return 0.0f;
    if (info->m_VisDist == 0.0f)
    {
        return 1.0f;
    }
    return std::min(1.0f,(info->m_VisDist + 20.0f - mid.length()) * 0.05f);
}
static void particle_AddSystemToFreeList(ParticleSys1 *sys)
{
    if (!sys)
        return;

    --g_particle_engine.dat.num_systems;
    sys->inuse = 0;
    if (sys == g_particle_engine.dat.all_lst)
    {
        g_particle_engine.dat.all_lst = sys->next;
        if (g_particle_engine.dat.all_lst == nullptr)
            assert(g_particle_engine.dat.num_systems == 0);
    }
    if (sys->next)
        sys->next->prev = sys->prev;
    if (sys->prev)
        sys->prev->next = sys->next;
    ParticleSys1 *head = g_particle_engine.dat.system_head;
    if (g_particle_engine.dat.system_head)
        g_particle_engine.dat.system_head->prev = sys;

    sys->next                         = head;
    sys->prev                         = nullptr;
    g_particle_engine.dat.system_head = sys;
}
void segs_releaseParticleSystem(ParticleSys1 *sys, int id)
{
    if (!sys || !sys->inuse || sys->unique_id != id)
        return;

    if (sys->Particle_VBO)
    {
        segs_wcw_statemgmt_FreeVBO(GL_ARRAY_BUFFER, 1, (GLuint *)&sys->Particle_VBO, "Particle_VBO");
    }
    COH_FREE(sys->particles);
    ParticleSys1 *bk_next = sys->next;
    ParticleSys1 *bk_prev = sys->prev;
    memset(sys, 0, sizeof(ParticleSys1));
    sys->next      = bk_next;
    sys->prev      = bk_prev;
    sys->inuse     = true; // the check above means that the system IS in use., TODO: consider why it is marked as such on release
    sys->unique_id = -1;
    particle_AddSystemToFreeList(sys);
}
void segs_partRunEngine()
{
    static int32_t g_highest_part_count;

    bool still_alive;
    ParticleSys1 *next_iter;
    GLDebugGuard debug_guard("segs_partRunEngine");
    g_particle_engine.systems_on = g_State.view.noParticles == 0;
    int parts_updated = 0;
    int parts_drawn = 0;
    int part_sys_updated = 0;
    int part_sys_drawn = 0;
    int32_t total_parts = 0;
    if (g_particle_engine.dat.num_systems && g_State.view.noParticles == 0)
    {
        MaterialDefinition part_mat(g_default_mat);
        part_mat.setDrawMode(DrawMode::DUALTEX);
        part_mat.setFragmentMode(eBlendMode::MULTIPLY);
        segs_rdrPrepareToRenderParticleSystems(part_mat);
        g_particle_engine.dat.all_lst = segs_partSortByDistance(g_particle_engine.dat.all_lst);
        float clampedTimestep = std::min(g_TIMESTEP,2.0f);
        for (ParticleSys1 *iter = g_particle_engine.dat.all_lst; iter; iter = next_iter)
        {
            next_iter = iter->next;
            if (iter->KickStart)
            {
                iter->currDrawState = 1;
                partKickStartSystem(iter);
            }
            if (!iter->parentFxDrawState || (iter->sysInfo->m_Flags & 1) || iter->age < iter->sysInfo->m_TimeToFull)
                iter->currDrawState = 0;
            else
                iter->currDrawState = 2;
            total_parts += iter->particle_count;
            if (g_State.view.fx_debug_req & 0x40)
            {
                still_alive = 1;
            }
            else if (iter->currDrawState == 2)
            {
                still_alive = systemIsStillAlive(iter, g_TIMESTEP);
            }
            else
            {
                still_alive = partUpdateSystem(iter, g_TIMESTEP, clampedTimestep);
                parts_updated += iter->particle_count;
                ++part_sys_updated;
            }
            if (still_alive)
            {
                if (!iter->particle_count || iter->currDrawState)
                {
                    iter->emission_start_prev = iter->emission_start;
                    iter->lastDrawMat = iter->drawMat;
                    iter->teleported = 0;
                    iter->m_MaxAlpha = 255;
                    continue;
                }
                Matrix4x3 *systemMatCamSpace;

                if (iter->sysInfo->m_FrontOrLocalFacing == 1)
                {
                    static Matrix4x3 systemMatCamSpaceStatic;
                    systemMatCamSpace = &systemMatCamSpaceStatic;
                    Matrix4x3 dest = iter->drawMat;
                    pitchMat3(glm::half_pi<float>(), &dest.ref3());
                    systemMatCamSpaceStatic = cam_info.viewmat * dest;
                }
                else
                {
                    systemMatCamSpace = nullptr;
                }
                float min_alpha = std::min(float(iter->m_MaxAlpha) / 255.0f,partCalculatePerformanceAlphaFade(iter));
                if ((g_State.view.stoprender_ifinactivedisplay && g_State.view.inactive_display) || (min_alpha <= 1.0f / 51.0f))
                {
                    iter->emission_start_prev = iter->emission_start;
                    iter->lastDrawMat         = iter->drawMat;
                    iter->teleported          = 0;
                    iter->m_MaxAlpha          = 255;
                    continue;
                }
                GLuint VBOBuffer = 0;
                if (iter->Particle_VBO)
                {
                    VBOBuffer = iter->Particle_VBO;
                }
                else if (iter->particle_count < 40)
                {
                    iter->rgbas = (void *)g_particle_engine.smallSystemRGBOffset;
                    VBOBuffer   = g_particle_engine.smallSystemVBOs[g_particle_engine.smallSystemsIdx++];
                    if (g_particle_engine.smallSystemsIdx >= 200)
                        g_particle_engine.smallSystemsIdx = 0;
                }
                else if (iter->particle_count < 300)
                {
                    iter->rgbas = (void *)g_particle_engine.u_3B4;
                    VBOBuffer   = g_particle_engine.mediumSystemVBOs[g_particle_engine.mediumSystemsIdx++];
                    if (g_particle_engine.mediumSystemsIdx >= 20)
                        g_particle_engine.mediumSystemsIdx = 0;
                }
                else if (iter->particle_count <= 1500)
                {
                    iter->rgbas = (void *)g_particle_engine.u_3B8;
                    VBOBuffer   = g_particle_engine.largeSystemVBOs[g_particle_engine.largeSystemsIdx++];
                    if (g_particle_engine.largeSystemsIdx >= 5)
                        g_particle_engine.largeSystemsIdx = 0;
                }
                else
                {
                    assert(false);
                }
                assert(VBOBuffer);
                parts_drawn += segs_modelDrawParticleSys(iter, min_alpha, VBOBuffer, systemMatCamSpace,part_mat);
                ++part_sys_drawn;
                iter->emission_start_prev = iter->emission_start;
                iter->lastDrawMat = iter->drawMat;
                iter->teleported = 0;
                iter->m_MaxAlpha = 255;
            }
            else
                segs_releaseParticleSystem(iter, iter->unique_id);
        }
        segs_rdrCleanUpAfterRenderingParticleSystems();
    }
    g_particle_engine.dat.particle_count = total_parts;
    if (g_highest_part_count < total_parts)
        g_highest_part_count = total_parts;
    if (g_State.view.resetparthigh)
    {
        g_highest_part_count = 0;
        font__489250();
        g_State.view.resetparthigh = 0;
    }
    if (g_State.view.fx_debug_req & 1)
    {
        xyprintf(0, 0x2030, "Particles Drawn    :%4d", parts_drawn);
        xyprintf(0, 0x2031, "Particles Updated  :%4d", parts_updated);
        xyprintf(0, 0x2032, "Particles Total    :%4d (High%4d)", total_parts, g_highest_part_count);
        xyprintf(0, 0x2033, "PartSys   Drawn    :%4d", part_sys_drawn);
        xyprintf(0, 0x2034, "PartSys   Updated  :%4d", part_sys_updated);
        xyprintf(0, 0x2035, "PartSys   Total    :%4d", g_particle_engine.dat.num_systems);
    }
    if (IsDevelopmentMode())
    {
        if (g_particle_engine.dat.num_systems > 7450u)
        {
            ErrorfFL(__FILE__, __LINE__);
            ErrorfInternal("PART: Soon Too many Systems!!!!!!!!!  %d", g_particle_engine.dat.num_systems);
        }
    }
}
static void buildPartSysList(ParticleSys1 *sys)
{
    g_particle_engine.dat.all_lst = nullptr;
    g_particle_engine.dat.num_systems = 0;

    for(int idx = 0; idx<7500; ++idx)
    {
        ParticleSys1 *iter = &sys[idx];
        iter->next = idx != 7499 ? &sys[idx + 1] : nullptr;
        iter->prev = idx != 0 ? iter - 1 : nullptr;
        iter->inuse = 0;
    }
    g_particle_engine.dat.system_head = sys;
}
static void  initPrecompTables()
{
    for (int i = 0; i < 512; ++i)
    {
        float angle = i * glm::pi<float>() / 128.0f;
        g_precomp_sin[i] = std::sin(angle);
        g_precomp_cos[i] = std::cos(angle);
    }
    g_precomp_sin[512] = g_precomp_sin[0];
    g_precomp_cos[512] = g_precomp_cos[0];
}

void segs_partEngineInitialize()
{
    if (g_State.view.noParticles)
        return;

    memset(&g_particle_engine, 0, sizeof(g_particle_engine));
    char *buf_mem = (char *)calloc(1, 96000); // used to initialize all buffers to 0
    g_particle_engine.smallSystemRGBOffset = 1920;
    glGenBuffers(200, g_particle_engine.smallSystemVBOs);
    for (uint32_t &buf_id : g_particle_engine.smallSystemVBOs) {
        glBindBuffer(GL_ARRAY_BUFFER, buf_id);
        glBufferData(GL_ARRAY_BUFFER, 2560, buf_mem, GL_STREAM_DRAW);
        glObjectLabel(GL_BUFFER, buf_id, -1, "Particle_Colors_Small_VBO");
    }
    g_particle_engine.u_3B4 = 14400;
    glGenBuffers(20, g_particle_engine.mediumSystemVBOs);
    for (uint32_t &buf_id : g_particle_engine.mediumSystemVBOs) {
        glBindBuffer(GL_ARRAY_BUFFER, buf_id);
        glBufferData(GL_ARRAY_BUFFER, 19200, buf_mem, GL_STREAM_DRAW);
        glObjectLabel(GL_BUFFER, buf_id, -1, "Particle_Colors_Medium_VBO");
    }
    g_particle_engine.u_3B8 = 72000;
    glGenBuffers(5, g_particle_engine.largeSystemVBOs);
    for (uint32_t &buf_id : g_particle_engine.largeSystemVBOs) {
        glBindBuffer(GL_ARRAY_BUFFER, buf_id);
        glBufferData(GL_ARRAY_BUFFER, 96000, buf_mem, GL_STREAM_DRAW);
        glObjectLabel(GL_BUFFER, buf_id, -1, "Particle_Colors_Large_VBO");
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    g_particle_engine.verts     = nullptr;
    g_particle_engine.u_3BC = nullptr; // coh was recording the buf_mem here, but it was not used anywhere.
    free(buf_mem);

    ParticleQuad *quad_mem = (ParticleQuad *)calloc(1, 1500*sizeof(ParticleQuad));
    for(int i =0; i<1500; ++i)
    {
        ParticleQuad &quad(quad_mem[i]);
        quad.v[0] = { 0,0 };
        quad.v[1] = { 1,0 };
        quad.v[2] = { 0,1 };
        quad.v[3] = { 1,1 };
    }
    glGenBuffers(1, &g_particle_engine.uv_coords_buf);
    glBindBuffer(GL_ARRAY_BUFFER, g_particle_engine.uv_coords_buf);
    glBufferData(GL_ARRAY_BUFFER, 1500*sizeof(ParticleQuad), quad_mem, GL_STATIC_DRAW);
    glObjectLabel(GL_BUFFER, g_particle_engine.uv_coords_buf, -1, "Particle_VBO_UV");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    free(quad_mem);
    g_particle_engine.particle_vertices = nullptr;
    int              start_idx         = 1;
    ParticleQuadIdx *quad_idx_mem      = (ParticleQuadIdx *)calloc(1, sizeof(ParticleQuadIdx) * 1500);
    for (int idx = 0; idx<1500; ++idx)
    {
        ParticleQuadIdx &qd(quad_idx_mem[idx]);
        qd.v[0] = Vector3i{ start_idx + 1,start_idx - 1,start_idx };
        qd.v[1] = Vector3i{ start_idx    ,start_idx + 2,start_idx + 1 };
        start_idx += 4;
    }
    glGenBuffers(1, &g_particle_engine.indices_buf_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_particle_engine.indices_buf_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1500*sizeof(ParticleQuadIdx), quad_idx_mem, GL_STATIC_DRAW);
    glObjectLabel(GL_BUFFER, g_particle_engine.indices_buf_id, -1, "Particle_IBO");
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    free(quad_idx_mem);
    g_particle_engine.particle_indices = nullptr;
    g_particle_engine.systems_on       = 1;
    buildPartSysList(g_particle_engine.particle_systems);
    initPrecompTables();
    srand(10);
}
static ParticleSys1 *partAddSystemNodes(int &allocated, int count)
{
    allocated = 0;
    if ( count < 1 )
        return nullptr;
    if (!g_particle_engine.dat.system_head)
    {
        assert(false);
        return nullptr;
    }
    auto iter = g_particle_engine.dat.system_head;
    ParticleSys1 *first_moved = iter;
    while(count--)
    {
        iter->inuse = true;
        allocated++;
        if(iter->next || count==0) // don't increment on the last entry
            break;
        iter = iter->next;
    }
    g_particle_engine.dat.system_head = iter->next;
    if ( iter->next )
        iter->next->prev = nullptr;
    ParticleSys1 *previous_start=g_particle_engine.dat.all_lst;
    iter->next = previous_start;
    if ( g_particle_engine.dat.all_lst != nullptr )
        previous_start->prev = iter;
    g_particle_engine.dat.num_systems += allocated;
    g_particle_engine.dat.all_lst = first_moved;
    return first_moved;
}
static Parse_PSystem *partGetSystemInfo(const char *name);
static Parse_PSystem *partGiveNewParameters(ParticleSys1 *system, int sys_id, const char *name)
{
    assert (system->power );
    if ( !(system->inuse && system->unique_id == sys_id && name) )
        return nullptr;
    Parse_PSystem *info = partGetSystemInfo(name);
    if(!info)
        return nullptr;
    system->sysInfo = info;
    system->KickStart = info->m_KickStart!=0;
    system->KillOnZero = info->m_KillOnZero!=0;
    float power_scale = system->power * 0.1f;
    assert(power_scale > 0 && power_scale <= 1.0f);
    system->m_NewPerFrame = lerp(info->m_NewPerFrame[1],info->m_NewPerFrame[0],power_scale);
    system->m_BurbleAmplitude = lerp(info->m_BurbleAmplitude[1],info->m_BurbleAmplitude[0],power_scale);
    system->m_Burst = int32_t(lerp<float>(info->m_Burst[1],info->m_Burst[0],power_scale) + 0.5f);
    system->m_Alpha_124 = uint8_t(lerp<float>(info->m_Alpha[1],info->m_Alpha[0],power_scale) + 0.5f);
    system->m_StartSize = lerp(info->m_StartSize[1],info->m_StartSize[0],power_scale);
    system->m_EndSize = lerp(info->m_EndSize[1],info->m_EndSize[0],power_scale);
    system->initialVelocity = lerp(info->m_InitialVelocity[1],info->m_InitialVelocity[0],power_scale);
    system->m_InitialVelocityJitter = lerp(info->m_InitialVelocityJitter[1],info->m_InitialVelocityJitter[0],power_scale);
    system->m_EmissionStartJitter = lerp(info->m_EmissionStartJitter[1],info->m_EmissionStartJitter[0],power_scale);
    return info;
}
ParticleSys1 *segs_partCreateSystem(const char *name, int *generated_id, char force_kickstart, int power)
{
    ParticleSys1 *res;
    Parse_PSystem *info;
    int allocated_nodes;

    if ( g_State.view.noParticles )
        return nullptr;
    res = partAddSystemNodes(allocated_nodes, 1);
    if ( !res || !allocated_nodes )
    {
        if ( IsDevelopmentMode() )
        {
            ErrorfFL(__FILE__,__LINE__);
            ErrorfInternal("Can't alloc %s. System Count %d Particle Count %d", name, g_particle_engine.dat.num_systems, g_particle_engine.dat.particle_count);
        }
        return nullptr;
    }
    ParticleSys1 *bak_next = res->next;
    bool bak_inuse = res->inuse;
    ParticleSys1 *bak_prev = res->prev;

    memset(res, 0, sizeof(ParticleSys1));
    res->next = bak_next;
    res->prev = bak_prev;
    res->inuse = bak_inuse;
    if ( g_particle_engine.system_unique_ids == -1 )
        g_particle_engine.system_unique_ids = 0;
    res->unique_id = g_particle_engine.system_unique_ids++;
    if ( generated_id )
        *generated_id = res->unique_id;
    res->power = power;
    if ( !power )
        res->power = 10;
    res->m_MaxAlpha = 255;
    if ( !partGiveNewParameters(res, res->unique_id, name) )
    {
        font_489280(1, "PART: Can't find %s", name);
        ErrorfFL(__FILE__, __LINE__);
        ErrorfInternal("PART: Can't find %s", name);
        segs_releaseParticleSystem(res, res->unique_id);
        return nullptr;
    }

    res->KickStart |= force_kickstart;
    res->drawMat = Unity_Matrix;
    info = res->sysInfo;
    float total_amount = info->m_FadeOutBy * res->m_NewPerFrame + (float)res->m_Burst;
    res->texarray = g_particle_engine.particle_vertices;
    res->tris = nullptr;
    res->verts = nullptr; // offset in engine buffer
    res->rgbas = nullptr; // offset in engine buffer, updated in runEngine
    res->maxVertParticles = int32_t(total_amount + 10.0f);
    if ( info->m_MoveScale != 0.0f )
        res->maxVertParticles += 200;
    if ( res->m_BurbleAmplitude != 0.0f )
        res->maxVertParticles += 200;
    res->maxVertParticles = std::min(1500,res->maxVertParticles);
    res->particles = (PSystem_1AC *)COH_MALLOC(sizeof(PSystem_1AC) * res->maxVertParticles);
    res->Particle_VBO = 0;
    return res;
}
static int comparePSystemByName(Parse_PSystem **a, Parse_PSystem **b)
{
    return strcasecmp((*a)->m_Name, (*b)->m_Name);
}
void segs_LoadParticles()
{
    char buf[300];

    utils_report_log_to_file1("Preloading particles..");
    ParserLoadFiles("fx", ".part", "particles.bin", 1, ParticleSystem_Token, &dword_7F8898, nullptr, nullptr, nullptr);
    int arr_size = COH_ARRAY_SIZE(dword_7F8898);
    for (int i = 0; i < arr_size; ++i )
    {
        Parse_PSystem *entry = dword_7F8898[i];
        segs_fxCleanFileName(buf, entry->m_Name);
        strcpy(entry->m_Name,buf);
    }
    qsort(dword_7F8898, (size_t)arr_size, sizeof(void *), (int (*)(const void *, const void *))comparePSystemByName);
    PrintfConsole1("");
}
void segs_resetListEntryContents(ListEntry *entry, size_t entry_size)
{
    if (!entry)
        return;
    ListEntry *bak_next = entry->next;
    ListEntry *bak_prev = entry->prev;
    memset(entry, 0, entry_size);
    entry->next = bak_next;
    entry->prev = bak_prev;
}
void closeAllOpenFileHandles(ParserRelatedStruct *parser)//fn_5BB120
{
    if ( !parser )
        return;

    for (ParserRel_Sub1 *iter = parser->current_sourcefile; iter; iter = parser->current_sourcefile )
    {
        parser->current_sourcefile = iter->parent_sourcefile;
        Destroy_XFileHandle(&iter->fh);
    }
    COH_FREE(parser);
}

static Parse_PSystem *parseSystemAndAddInfoToList(char *file_to_parse)
{
    Parse_PSystem *sysInfo = nullptr;
    char buf[1000];

    ParserRelatedStruct *tokenizer = CreateTokenizer(file_to_parse);
    if ( tokenizer )
    {
        sysInfo = (Parse_PSystem *)alloc_and_add_to_list((ListEntry *)&g_particle_engine.first_parsed, sizeof(Parse_PSystem), __FILE__, __LINE__);
        assert( sysInfo );
        segs_resetListEntryContents((ListEntry *)sysInfo, sizeof(Parse_PSystem));
        if ( fn_5BE550(tokenizer, ParticleTokens, (char *)sysInfo, UnrecognizedToken) )
        {
            segs_fxCleanFileName(buf, sysInfo->m_Name);
            strcpy(sysInfo->m_Name,buf);
            ++g_particle_engine.count_parsed;
        }
        else
        {
            free_and_remove_from_list((ListEntry *)sysInfo, (ListEntry **)&g_particle_engine.first_parsed);
            sysInfo = nullptr;
        }
    }
    closeAllOpenFileHandles(tokenizer);
    return sysInfo;
}
static int fxPartNameCmp2(Parse_PSystem *a, Parse_PSystem *b)
{
    return strcasecmp(a->m_Name, b->next->m_Name);
}
static Parse_PSystem *partGetSystemInfo(const char *sysname)
{
    Parse_PSystem *sysinfo= nullptr;
    char clean_name[304];
    char lpFileName[500];
    Parse_PSystem to_find;

    memset(&to_find, 0, sizeof(to_find));

    segs_fxCleanFileName(clean_name, sysname);
    to_find.m_Name = clean_name;
    int count = COH_ARRAY_SIZE(dword_7F8898 );
    Parse_PSystem **sysinfo_iter = (Parse_PSystem **)bsearch(&to_find, dword_7F8898, count, 4u, (int (*)(const void *, const void *))fxPartNameCmp2);
    if ( sysinfo_iter )
    {
        sysinfo = *sysinfo_iter;
        assert( sysinfo );
        if ( IsDevelopmentMode() )
            sysinfo->last_file_change = fileLastChanged("bin/particles.bin");
    }
    if ( IsDevelopmentMode() )
    {
        for(Parse_PSystem *iter = g_particle_engine.first_parsed; iter; iter = iter->next)
        {
            if(0==strcasecmp(clean_name,iter->m_Name))
            {
                sysinfo = iter;
                break;
            }
        }
        sprintf(lpFileName, "%s%s", "fx/", clean_name);
        if ( !sysinfo || fileHasBeenUpdated(lpFileName, &sysinfo->last_file_change) )
            sysinfo = parseSystemAndAddInfoToList(lpFileName);
    }
    if ( sysinfo )
    {
        if ( !sysinfo->initialized )
        {
            partInitSystemInfo(sysinfo);
            sysinfo->initialized = 1;
        }
        if ( IsDevelopmentMode() )
        {
            sysinfo->last_file_change = 0;
            sprintf(lpFileName, "%s%s", "fx/", clean_name);
            fileHasBeenUpdated(lpFileName, &sysinfo->last_file_change);
            return sysinfo;
        }
    }
    else
    {
        font_489280(1, "PART: Failed to get system %s", sysname);
        if ( g_particle_engine.first_parsed )
        {
            for(auto iter=g_particle_engine.first_parsed; iter; iter= iter->next)
            {
                if ( !iter->m_Name || !strcasecmp(clean_name, iter->m_Name) )
                {
                    free_and_remove_from_list((ListEntry *)iter,(ListEntry **)&g_particle_engine.first_parsed);
                    --g_particle_engine.count_parsed;
                    assert(g_particle_engine.count_parsed>=0);
                }
            }
        }
        if ( g_State.view.fx_debug_req & 1 )
        {
            fprintf(stderr,"Particle system %s doesn't exist, we'll need to fix the assets too ??\n", sysname);
            assert(false);
        }
    }
    return sysinfo;
}
void patch_renderparticles()
{
    BREAK_FUNC(rdrCleanUpAfterRenderingParticleSystems);
    patchit("particle_InitializeSystemAddToEngine", reinterpret_cast<void *>(segs_releaseParticleSystem));
    PATCH_FUNC(partEngineInitialize);
    PATCH_FUNC(LoadParticles);
    PATCH_FUNC(partCreateSystem);
    PATCH_FUNC(resetListEntryContents);
    patchit("fn_5BB120",reinterpret_cast<void *>(closeAllOpenFileHandles));
    BREAK_FUNC(partRunEngine);

}
