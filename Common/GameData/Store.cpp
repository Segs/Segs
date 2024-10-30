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
#include "EASTL/algorithm.h"
#include "cereal/cereal.hpp"

StoreTransactionResult Store::buyItem(Entity *e, StringView item_name)
{
    const GameDataStore &data(getGameData());
    StoreTransactionResult result;

    auto found=eastl::find_if(data.m_shop_items_data.begin(),data.m_shop_items_data.end(),[&](const ShopItemInfo_Data &e)->bool {
        return e.m_Name==item_name;
    });

    if(found==data.m_shop_items_data.end()) {
        result.m_is_success=false;
        result.m_message="No such item";
        sCDebug(logStores) << "buyItem. Item not found";
        return result;
    }
    const ShopItemInfo_Data &item_info = *found;

    sCDebug(logStores) << "buyItem. Item found";
    int price = getPrice(e, item_name, false);
    result.m_inf_amount = price;
    result.m_is_sell = false;

    if(item_info.m_Power.m_PowerCategory == "Inspirations")
    {
        String insp_name = item_info.m_Name;
        String message = String(String::CtorSprintf(),"Bought %s for %2 Influence",insp_name.c_str(),abs(price));

        result.m_is_insp = true;
        result.m_item_name = insp_name;
        result.m_message = message;
        result.m_is_success = true;

        sCDebug(logStores) << "buyItem. Inspiration: " << insp_name;
    }
    else
    {
        auto ending_index = item_name.rfind("_")+1;
        String name = String(item_name).substr(0,ending_index - 1);
        String num = String(item_name).substr(ending_index, item_name.length() - ending_index);

        result.m_item_name = name;
        result.m_enhancement_lvl = std::stoi(num.c_str());
        result.m_is_success = true;
        result.m_message = String(String::CtorSprintf(),"Bought %s for %d Influence",name.c_str(),abs(price));

        sCDebug(logStores) << "buyItem. Enhancement: " << name << " lvl: " << num;
    }
    return result;
}

StoreTransactionResult Store::sellItem(Entity *e, StringView item_name)
{
    StoreTransactionResult result;
    sCDebug(logStores) << "sellItem. Item to find." << item_name;

    int price = getPrice(e, item_name, true);
    result.m_message = String(String::CtorSprintf(),"Sold %.*s for %d influence.",item_name.size(),item_name.data(),price);
    result.m_inf_amount = price;
    result.m_is_success = true;
    result.m_item_name = item_name;

    return result;
}

int Store::getPrice(Entity *e, StringView item_name, bool is_selling)
{
    const GameDataStore &data(getGameData());
    Vector<Shop_Data> shop_data; // NPC could have multiple shop_names set
    if(e->m_is_store && !e->m_store_items.empty())
    {
        //Find store in entity store list
        for(const StoreItem &si: e->m_store_items)
        {
            for(const Shop_Data &shop: data.m_shops_data)
            {
                if(si.m_store_name == shop.m_Name)
                {
                    shop_data.emplace_back(shop);
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
                sCDebug(logStores) << "Item " << item_name << " found";
                break;
            }
        }

        for (uint32_t dept: item_info.m_Departments)
        {
            for (const Shop_Data &shop: shop_data)
            {
                //Find the dept to get markup for item.
                if(is_selling)
                {
                    // Buy and Sell variable names are reversed in bin
                    for (const ShopBuySell_Data &sd: shop.m_Buys)
                    {
                        sCDebug(logStores) << "Get price to sell. sd.Dept: " <<sd.m_Department << " dept: " << dept;
                        if(sd.m_Department == dept)
                        {
                            sCDebug(logStores) << "Item to sell price: " << item_info.m_Buy * sd.m_Markup;

                            return item_info.m_Buy * sd.m_Markup;
                        }
                    }
                }
                else
                {
                     // Buy and Sell variable names are reversed in bin
                    for (const ShopBuySell_Data &sd: shop.m_Sells)
                    {
                        sCDebug(logStores) << "Get price to Buy. sd.Dept: " <<sd.m_Department << " dept: " << dept;
                        if(sd.m_Department == dept)
                        {
                            sCDebug(logStores) << "Item to buy price: " << -(item_info.m_Sell * sd.m_Markup);
                            // Buy and Sell variable names are reversed in bin
                            return -(item_info.m_Sell * sd.m_Markup);
                        }
                    }
                }
            }
        }
    }

    sCDebug(logStores) << "Item not found.";
    return 0; // Not found
}


template<class Archive>
void Store::serialize(Archive &archive, uint32_t const version)
{
    if(version != Store::class_version)
    {
        sCritical() << "Failed to serialize Store, incompatible serialization format version " << version;
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
        sCritical() << "Failed to serialize StoreItem, incompatible serialization format version " << version;
        return;
    }
    archive(cereal::make_nvp("itemName",m_store_name));
    archive(cereal::make_nvp("itemCount",m_item_count));
}

CEREAL_CLASS_VERSION(StoreItem, StoreItem::class_version)   // register StoreItem class version
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(StoreItem)
