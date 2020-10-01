/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Handle.h"
#include <cassert>
#include <vector>

template <int idx_bits, int gen_bits, typename T>
struct SingularStoreHandleT : public HandleT<idx_bits, gen_bits,T>
{
    using Type = T;
    using Storage = typename T::StorageClass;
    using super = HandleT<idx_bits, gen_bits,T>;
    using super::super;
    // allow initializing SingularStorage handles from underlying type
    constexpr SingularStoreHandleT(super from) : super(from) {}
    constexpr SingularStoreHandleT() : super() {}
    T*  operator->() const { return &Storage::instance().access(*this); }
    T &  get() const { return Storage::instance().access(*this); }
    void destroy() { Storage::instance().destroy(*this); }
};
template <class T>
struct HandleBasedStorage
{
    using HType          = SingularStoreHandleT<20,12,T>;
    using InternalHandle = Handle<20, 12>;
    using container_type = std::vector<T>;
    using iterator       = typename container_type::iterator;

    template <typename... Args>
    HType create(Args &&... args)
    {
        uint32_t internal_idx = uint32_t(m_nodes.size());
        m_nodes.emplace_back(std::forward<Args>(args)...);
        uint32_t node_idx        = getNextFreeNode();
        m_free_list_head         = m_sparse_array[m_free_list_head].idx;
        m_sparse_array[node_idx] = HType(internal_idx, m_sparse_array[node_idx].gen);
        m_dense_to_sparse.emplace_back(node_idx);
        return HType(node_idx, m_sparse_array[node_idx].gen);
    }
    void destroy(HType n)
    {
        assert(n.idx < m_sparse_array.size());
        InternalHandle &h(m_sparse_array[n.idx]);
        assert(n.gen == h.gen);
        // mark as deleted in sparse array
        h.gen++;
        // swap and pop
        m_dense_to_sparse[h.idx] = m_dense_to_sparse.back();
        m_dense_to_sparse.pop_back();
        m_sparse_array[m_dense_to_sparse[h.idx]].idx = h.idx;
        m_nodes[h.idx].~T();
        new (&m_nodes[h.idx]) T(std::move(m_nodes.back()));
        addToFreeList(n.idx);
        m_nodes.pop_back();
    }
    size_t liveCount() const { return m_nodes.size(); }
    bool canAccess(HType n) { return m_sparse_array[n.idx].gen == n.gen; }
    T &  access(HType n)
    {
        assert(canAccess(n));
        return m_nodes[m_sparse_array[n.idx].idx];
    }
    iterator begin() { return m_nodes.begin(); }
    iterator end() { return m_nodes.end(); }

    HType handle_for_entry(const T &entry) const
    {
        int expected_idx=std::distance(m_nodes.data(),&entry);
        assert(expected_idx>=0);
        assert(expected_idx<m_nodes.size());
        return HType {uint32_t(expected_idx),m_sparse_array[expected_idx].gen};
    }

    HandleBasedStorage()
    {
        m_sparse_array.reserve(256);
        m_nodes.reserve(256);
        m_dense_to_sparse.reserve(256);
    }
private:

    std::vector<InternalHandle> m_sparse_array;
    uint32_t                    m_free_list_head = HType::FREE_LIST_TERMINATOR;
    container_type              m_nodes;
    std::vector<uint32_t>       m_dense_to_sparse;

    void addToFreeList(uint32_t node_idx)
    {
        m_sparse_array[node_idx].idx = m_free_list_head;
        m_free_list_head             = node_idx;
    }
    uint32_t getNextFreeNode()
    {
        if(m_free_list_head != HType::FREE_LIST_TERMINATOR)
            return m_free_list_head;
        //NOTE: we allocate 256 new 'nodes' each time
        uint32_t start_idx = m_sparse_array.size();
        assert(start_idx + 256 < HType::FREE_LIST_TERMINATOR);
        m_free_list_head = start_idx;
        m_sparse_array.resize(start_idx + 256);
        for (auto iter = m_sparse_array.begin() + start_idx, fin = m_sparse_array.end(); iter != fin; ++iter)
        {
            iter->idx = ++start_idx;
            iter->gen = 1;
        }
        m_sparse_array.back().idx = HType::FREE_LIST_TERMINATOR;
        return m_free_list_head;
    }
};

