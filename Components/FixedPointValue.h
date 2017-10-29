#pragma once
#include <stdint.h>
#include <cmath>
#include <cassert>
struct FixedPointValue
{
    int32_t store;
    constexpr FixedPointValue() : store(0x800000) {}
    constexpr FixedPointValue(float v) : store(int(v * 64.0f) + 0x800000)
    {
        assert(!std::isnan(v));
        assert(std::abs(v)*64 <0x800000);
    }
    explicit operator float() const { return (store-0x800000) / 64.0f; }
};

struct Vector3_FPV
{
    FixedPointValue x,y,z;
};
