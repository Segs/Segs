/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "Store.h"
#include "Entity.h"
#include "GameDataStore.h"
#include "Components/Logging.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include "cereal/cereal.hpp"

StoreTransactionResult Store::buyItem(Entity *e, QString item_name)
{
    const GameDataStore &data(getGameData());
    StoreTransactionResult result;

    for(const ShopItemInfo_Data &item_info: data.m_shop_items_data)
    {
        if(item_info.m_Name == item_name)
        {
            qCDebug(logStores) << "buyItem. Item found";
            int price = getPrice(e, item_name, false);
            result.m_inf_amount = price;
            result.m_is_sell = false;

            if(item_info.m_Power.m_PowerCategory == "Inspirations")
            {
                QString insp_name = item_info.m_Name;
                QString message = QString("Bought %1 for %2 Influence").arg(insp_name).arg(abs(price));

                result.m_is_insp = true;
                result.m_item_name = insp_name;
                result.m_message = message;
                result.m_is_success = true;

                qCDebug(logStores) << "buyItem. Inspiration: " << insp_name;
            }
            else
            {
                QVector<QStringRef> parts;
                int ending_index = item_name.lastIndexOf("_")+1;
                parts.push_back(item_name.midRef(0,ending_index - 1));
                parts.push_back(item_name.midRef(ending_index, item_name.length() - ending_index));

                QString name = parts[0].toString();
                result.m_item_name = name;
                result.m_enhancement_lvl = parts[1].toInt();
                result.m_is_success = true;
                result.m_message = QString("Bought %1 for %2 Influence").arg(name).arg(abs(price));

                qCDebug(logStores) << "buyItem. Enhancement: " << name << " lvl: " << parts[1].toString();
            }
            break;
        }
    }

    return result;
}

StoreTransactionResult Store::sellItem(Entity *e, QString item_name)
{
    StoreTransactionResult result;
    qCDebug(logStores) << "sellItem. Item to find." << item_name;

    int price = getPrice(e, item_name, true);
    result.m_message = QString("Sold %1 for %2 influence.").arg(item_name).arg(price);
    result.m_inf_amount = price;
    result.m_is_success = true;
    result.m_item_name = item_name;

    return result;
}

int Store::getPrice(Entity *e, QString item_name, bool is_selling)
{
    const GameDataStore &data(getGameData());
    std::vector<Shop_Data> shop_data; // NPC could have multiple shop_names set
    if(e->m_is_store && !e->m_store_items.empty())
    {
        //Find store in entity store list
        for(const StoreItem &si: e->m_store_items)
        {
            for(const Shop_Data &shop: data.m_shops_data)
            {
                if(si.m_store_name == shop.m_Name)
                {
                    shop_data.push_back(shop);
                    break;
                }
            }
        }

        //Find item to sell item info
        ShopItemInfo_Data item_info;
        for(const ShopItemInfo_Data &iteminfo: data.m_shop_items_data)
        {
            if(iteminfo.m_Name == item_name)
            {
                item_info = iteminfo;
                qCDebug(logStores) << "Item " << item_name << " found";
                break;
            }
        }

        for (const int &dept: item_info.m_Departments)
        {
            for (const Shop_Data &shop: shop_data)
            {
                //Find the dept to get markup for item.
                if(is_selling)
                {
                    // Buy and Sell variable names are reversed in bin
                    for (const ShopBuySell_Data &sd: shop.m_Buys)
                    {
                        qCDebug(logStores) << "Get price to sell. sd.Dept: " <<sd.m_Department << " dept: " << dept;
                        if(sd.m_Department == dept)
                        {
                            qCDebug(logStores) << "Item to sell price: " << item_info.m_Buy * sd.m_Markup;

                            return item_info.m_Buy * sd.m_Markup;
                        }
                    }
                }
                else
                {
                     // Buy and Sell variable names are reversed in bin
                    for (const ShopBuySell_Data &sd: shop.m_Sells)
                    {
                        qCDebug(logStores) << "Get price to Buy. sd.Dept: " <<sd.m_Department << " dept: " << dept;
                        if(sd.m_Department == dept)
                        {
                            qCDebug(logStores) << "Item to buy price: " << -(item_info.m_Sell * sd.m_Markup);
                            // Buy and Sell variable names are reversed in bin
                            return -(item_info.m_Sell * sd.m_Markup);
                        }
                    }
                }
            }
        }
    }

    qCDebug(logStores) << "Item not found.";
    return 0; // Not found
}


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
