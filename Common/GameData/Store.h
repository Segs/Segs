/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/GameData/CommonNetStructures.h"
#include "Components/Logging.h"
#include "cereal/cereal.hpp"

class Entity;

class StoreTransactionResult
{
public:
    bool                m_is_success = false;
    bool                m_is_sell = false;
    bool                m_is_insp = false;
    QString             m_item_name;
    int                 m_inf_amount;
    uint32_t            m_enhancement_lvl;
    QString             m_message;
};


class StoreItem
{
public:
    enum : uint32_t {class_version       = 1};
    QString     m_store_name;
    int         m_item_count;

    StoreItem(){}
    StoreItem(QString item_name, int item_count)
    {
        m_store_name = item_name;
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

    static StoreTransactionResult buyItem(Entity *e, QString item_name);
    static StoreTransactionResult sellItem(Entity *e, QString item_name);
    static int getPrice(Entity *e, QString item_name, bool is_selling);
};




