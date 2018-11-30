/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */


#include "CommonNetStructures.h"
#include "cereal/cereal.hpp"
#include "Logging.h"


class Store
{
public:
    enum : uint32_t {class_version       = 1};

    uint32_t           m_npc_idx;
    std::vector<QString>        m_store_items;
    size_t             m_count;
    std::vector<int>   m_item_count; // Item cost?


    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);

};
