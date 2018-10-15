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
    Handle(uint32_t idx_, uint16_t gen_) : idx(idx_), gen(gen_) {}
    Handle() = default;
    operator bool() const { return gen != 0; }
};

template <int idx_bits, int gen_bits, typename T>
struct HandleT : public Handle<idx_bits, gen_bits>
{
    using Type = T;
    using Handle<idx_bits, gen_bits>::Handle;
};
