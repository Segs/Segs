#pragma once
#pragma pack(push, 1)
struct GfxPrefs
{
  int screen_x;
  int screen_y;
  int screenX_pos;
  int screenY_pos;
  int maximized;
  int fullscreen;
  int mipLevel;
  float controls_draw_dist;
  float LODBias;
  float gamma;
  int disableSimpleShadows;
  int maxParticles;
  float maxParticleFill_div1mln;
  float fxSoundVolume;
  float musicSoundVolume;
  int enableVBOs;
};
#pragma pack(pop)

extern void patch_gfx();
