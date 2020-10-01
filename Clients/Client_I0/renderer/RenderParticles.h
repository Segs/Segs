#pragma once
#include "utils/helpers.h"
#include <GL/glew.h>

struct PSystem_1AC
{
    Vector3 m_pos;
    Vector3 m_velocity;
    float age;
    uint8_t alpha;
    uint8_t theta;
    int16_t spin;
    float size;
    int pulse_direction;
};
static_assert(sizeof(PSystem_1AC) == 0x28);
struct ParticleSys1
{
    ParticleSys1 *next;
    ParticleSys1 *prev;
    char inuse;
    int unique_id;
    struct Parse_PSystem *sysInfo;
    Matrix4x3 facingMat;
    Vector3 emission_start;
    Vector3 emission_start_prev;
    Vector3 other_vec;
    Vector3 magnet_vector;
    Matrix4x3 drawMat;
    Matrix4x3 lastDrawMat;
    int particle_count;
    float age;
    int parentFxDrawState;
    int currDrawState;
    float new_buffer;
    char KickStart;
    int dying;
    float m_NewPerFrame;
    float m_BurbleAmplitude;
    int KillOnZero;
    int m_Burst;
    Vector3 m_EmissionStartJitter;
    Vector3 m_InitialVelocityJitter;
    Vector3 initialVelocity;
    uint8_t m_Alpha_124;
    char teleported;
    float m_StartSize;
    float m_EndSize;
    int power;
    Vector3 stickinessVec;
    float timeSinceLastUpdate;
    float vec_sq_length;
    Vector3 boxMax;
    Vector3 boxMin;
    float biggestParticle;
    float totalSize;
    float totalAlpha;
    Vector2 m_perTextureTranslate[2];
    float framecurr[2];
    int animpong;
    void *verts;
    void *rgbas;
    struct ParticleQuad *texarray;
    int *tris;
    GLint Particle_VBO;
    int maxVertParticles;
    int maxParticles;
    int firstParticle;
    uint8_t m_MaxAlpha;
    uint8_t unkn_1A9;
    struct PSystem_1AC *particles;
};
static_assert(sizeof(ParticleSys1) == 0x1B0);

struct Particle_Engine
{
    int u_0;
    float u_4;
    int particle_count;
    int u_C;
    ParticleSys1 *all_lst;
    uint16_t num_systems;
    int16_t u_16;
    ParticleSys1 *system_head;
};

struct ParticleGlobal
{
    int systems_on;
    int system_unique_ids;
    GLuint indices_buf_id;
    GLvoid *particle_indices;
    GLuint uv_coords_buf;
    struct ParticleQuad *particle_vertices;
    void *verts;
    void *u_1C;
    GLuint smallSystemVBOs[200];
    GLuint mediumSystemVBOs[20];
    GLuint largeSystemVBOs[5];
    int smallSystemsIdx;
    int mediumSystemsIdx;
    int largeSystemsIdx;
    int smallSystemRGBOffset; // offset of small buffer area
    int u_3B4; // offset of medium buffer area
    int u_3B8;
    void *u_3BC;
    Particle_Engine dat;
    ParticleSys1 particle_systems[7500];
    Parse_PSystem *first_parsed;
    int count_parsed;
};
static_assert (sizeof(ParticleGlobal) == 0x317424, "");

void segs_releaseParticleSystem(ParticleSys1 *sys, int id);
void segs_partRunEngine();
ParticleSys1 *segs_partCreateSystem(const char *name, int *generated_id, char force_kickstart, int power);

extern void patch_renderparticles();
