/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
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
    constexpr Handle(uint32_t idx_, uint16_t gen_) : idx(idx_), gen(gen_) {}
    constexpr Handle() = default;
    operator bool() const { return gen != 0; }
};

template <typename T,int idx_bits=20, int gen_bits=12>
struct HandleT : public Handle<idx_bits, gen_bits>
{
    using Type = T;
    using Handle<idx_bits, gen_bits>::Handle;
};
#if 0
template <typename T>
struct RefCountedHandle;
template<class T>
void addRef(RefCountedHandle<T> h);
template<class T>
void decRef(RefCountedHandle<T> h);

template <typename T>
struct RefCountedHandle : public HandleT<T>
{
    using super = HandleT<T>;

    constexpr RefCountedHandle() : super() {}
    constexpr RefCountedHandle(uint32_t idx_, uint16_t gen_) : super(idx_,gen_) {}
    RefCountedHandle(const RefCountedHandle &from) : super(from) {addRef(*this);}
    RefCountedHandle(const super &from) : super(from) {addRef(*this);}
    RefCountedHandle &operator=(const RefCountedHandle &from)
    {
        if(*this==from)
            return *this;
        addRef(from);
        decRef(*this);
        this->idx = from.idx;
        this->gen = from.gen;
        return *this;
    }
    RefCountedHandle &operator=(RefCountedHandle &&from)
    {
        if(*this==from)
            return *this;
        decRef(*this);
        this->idx = from.idx;
        this->gen = from.gen;
        from.idx = from.gen = 0;
        return *this;
    }
    ~RefCountedHandle() { decRef(*this); }
};
#endif
