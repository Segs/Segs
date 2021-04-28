/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <stdint.h>

struct ARGB
{
    union
    {
        uint8_t v[4];
        uint32_t val;
        struct { uint8_t a,r,g,b;} argb;
    };
    ARGB() {}
    ARGB(uint32_t _v) { val =_v;}
};
struct URG {
    uint32_t v;
    URG & operator=(uint32_t x) {
        v=x;
        return *this;
    }
    URG(uint32_t _clr=0) : v(_clr) {}
    URG(uint8_t r,uint8_t g,uint8_t b,uint8_t a) {
        v =  uint32_t(r) |
            (uint32_t(g)<<8) |
            (uint32_t(b)<<16) |
            (uint32_t(a)<<24);
    }
    uint8_t R() const { return v & 0xFF; }
    uint8_t G() const { return (v>>8) & 0xFF; }
    uint8_t B() const { return (v>>16) & 0xFF; }
    uint8_t A() const { return (v>>24) & 0xFF; }

};
#pragma pack(push,1)
struct RGB
{
    uint8_t R,G,B;
    uint8_t &operator[](size_t idx)
    {
        assert(idx<3);
        if(idx==0)
            return R;
        if(idx==1)
            return G;
        return B;
    }
};
#pragma pack(pop)
static_assert(sizeof(RGB)==3,"sizeof(RGB)==3");

struct RGBA
{
    union
    {
        uint8_t v[4];
        uint32_t val;
        struct { uint8_t r,g,b,a;} rgba;
        struct { uint8_t x,y,z,w;} xyzw ;
    };
    RGBA(uint8_t r_,uint8_t g_,uint8_t b_,uint8_t a_) {
        rgba.r = r_;
        rgba.g = g_;
        rgba.b = b_;
        rgba.a = a_;
    }
    RGBA() {}
    RGBA(uint32_t v)  {
        rgba.a = v & 0xFF;
        v>>=8;
        rgba.b = v & 0xFF;
        v>>=8;
        rgba.g = v & 0xFF;
        v>>=8;
        rgba.r = v & 0xFF;
    }
    RGBA &operator=(uint32_t v) {
        rgba.a = v & 0xFF;
        v>>=8;
        rgba.b = v & 0xFF;
        v>>=8;
        rgba.g = v & 0xFF;
        v>>=8;
        rgba.r = v & 0xFF;
        return *this;
    }
    RGBA &operator=(RGB v) {
        rgba.b = v.B;
        rgba.g = v.G;
        rgba.r = v.R;
        return *this;
    }
    bool rgb_are_zero() const { return !(rgba.r|rgba.g|rgba.b); }
    uint8_t &operator[](uint8_t idx) { return v[idx];}
    glm::vec4 toFloats() const { return glm::vec4(
                    rgba.r/255.0f,rgba.g/255.0f,rgba.b/255.0f,rgba.a/255.0f); }
    glm::vec3 to3Floats() const { return glm::vec3(
                    rgba.r/255.0f,rgba.g/255.0f,rgba.b/255.0f); }
    RGB toRGB() const { return {rgba.r,rgba.g,rgba.b}; }
};
static_assert(sizeof(RGBA)==4,"sizeof(RGBA)==4");
