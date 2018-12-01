/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */


#include "Store.h"
#include "cereal/cereal.hpp"
#include "Logging.h"
#include "serialization_common.h"
#include "serialization_types.h"

template<class Archive>
void Store::serialize(Archive &archive, uint32_t const version)
{
    if(version != Store::class_version)
    {
        qCritical() << "Failed to serialize Store, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("npcIdx",m_npc_idx));
    archive(cereal::make_nvp("storeItems", m_store_Items));

}
CEREAL_CLASS_VERSION(Store, Store::class_version)  // register Store class version
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(Store)

template<class Archive>
void StoreItem::serialize(Archive &archive, uint32_t const version)
{
    if(version != StoreItem::class_version)
    {
        qCritical() << "Failed to serialize StoreItem, incompatible serialization format version " << version;
        return;
    }
    archive(cereal::make_nvp("itemName",m_store_name));
    archive(cereal::make_nvp("itemCount",m_item_count));
}

CEREAL_CLASS_VERSION(StoreItem, StoreItem::class_version)   // register StoreItem class version
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(StoreItem)
