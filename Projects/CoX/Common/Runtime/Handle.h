/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <stdint.h>

template <int idx_bits, int gen_bits>
struct Handle
{
    enum
    {
        FREE_LIST_TERMINATOR = (1U << idx_bits) - 1
    };
    static_assert(sizeof(uint32_t) * 8 >= (idx_bits + gen_bits), "Handle bits exceed the size of underlying type");
    uint32_t idx : idx_bits;
    uint32_t gen : gen_bits;
    constexpr Handle(uint32_t idx_, uint32_t gen_) : idx(idx_), gen(gen_) {}
    constexpr Handle() = default;
    operator bool() const { return gen != 0; }
    uint64_t toInt() const { return uint64_t(idx) | uint64_t(gen) << idx_bits; }
    static Handle fromInt(uint64_t val) { return { val & ((1<<idx_bits)-1), (val>>idx_bits) & ((1 << gen_bits) - 1) }; }
};

template <int idx_bits, int gen_bits, typename T>
struct HandleT : public Handle<idx_bits, gen_bits>
{
    using Type = T;
    using Handle<idx_bits, gen_bits>::Handle;
};
