/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
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
        struct { uint8_t a,r,g,b;};
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
    //my_groupfileload_4C2460 thinks v&0xFF is alpha
    //my_light_493560 thinks otherwise ??
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
        struct { uint8_t r,g,b,a;};
        struct { uint8_t x,y,z,w;};
    };
    RGBA(uint8_t r_,uint8_t g_,uint8_t b_,uint8_t a_) : r(r_),g(g_),b(b_),a(a_) {}
    RGBA() {}
    RGBA(uint32_t v)  {
        a = v & 0xFF;
        v>>=8;
        b = v & 0xFF;
        v>>=8;
        g = v & 0xFF;
        v>>=8;
        r = v & 0xFF;
    }
    RGBA &operator=(uint32_t v) {
        a = v & 0xFF;
        v>>=8;
        b = v & 0xFF;
        v>>=8;
        g = v & 0xFF;
        v>>=8;
        r = v & 0xFF;
        return *this;
    }
    RGBA &operator=(RGB v) {
        b = v.B;
        g = v.G;
        r = v.R;
        return *this;
    }
    bool rgb_are_zero() const { return !(r|g|b); }
    uint8_t &operator[](uint8_t idx) { return v[idx];}
    glm::vec4 toFloats() const { return glm::vec4(r/255.0f,g/255.0f,b/255.0f,a/255.0f); }
    glm::vec3 to3Floats() const { return glm::vec3(r/255.0f,g/255.0f,b/255.0f); }
    RGB toRGB() const { return {r,g,b}; }
};

//static_assert(offsetof(RGBA,r)==0,"");
//static_assert(offsetof(RGBA,g)==1,"");
//static_assert(offsetof(RGBA,b)==2,"");
//static_assert(offsetof(RGBA,a)==3,"");

static_assert(sizeof(RGBA)==4,"sizeof(RGBA)==4");
