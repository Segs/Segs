/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <stdint.h>
#include <cmath>
#include <cassert>
#include <glm/vec3.hpp>

struct FixedPointValue
{
    int32_t store;
    constexpr FixedPointValue() : store(0x800000) {}
    constexpr explicit FixedPointValue(float v) : store(int(v * 64.0f) + 0x800000)
    {
#ifndef _MSC_VER
        assert(!std::isnan(v));
        assert(std::abs(v)*64 <0x800000);
#endif
    }
    explicit operator float() const { return (store-0x800000) / 64.0f; }
};

struct Vector3_FPV
{
    FixedPointValue x,y,z;
    Vector3_FPV() = default;
    Vector3_FPV(glm::vec3 from) : x(from.x),y(from.y),z(from.z) {}
};
