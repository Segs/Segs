/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "CommonNetStructures.h"
#include "cereal/cereal.hpp"
#include "Logging.h"


class StoreItem
{
public:
    enum : uint32_t {class_version       = 1};
    QString     m_item_name;
    int         m_item_count;

    StoreItem(){}
    StoreItem(QString item_name, int item_count)
    {
        m_item_name = item_name;
        m_item_count = item_count;
    }

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);

};
using vStoreItems = std::vector<StoreItem>;

class Store
{
public:
    enum : uint32_t {class_version       = 1};

    uint32_t           m_npc_idx;
    vStoreItems        m_store_Items;

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);

};



