/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "shop_serializers.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include "Common/GameData/shop_definitions.h"
#include "DataStorage.h"


bool loadFrom(BinStore *s, ShopBuySell_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Department);
    ok &= s->read(target.m_Markup);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    return s->end_encountered();
}

bool loadFromI24(BinStore *s, ShopBuySell_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Department);
    ok &= s->read(target.m_Markup);
    return ok;
}


bool loadFrom(BinStore *s, ShopItem_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Name);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    return s->end_encountered();
}

bool loadFromI24(BinStore *s, ShopItem_Data &target)
{
    s->prepare();
    bool ok = s->read(target.m_Name);
    return ok;
}

bool loadFrom(BinStore *s, ShopDeptName_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Names);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    return s->end_encountered();
}

static bool loadFromI24(BinStore *s, ShopDeptName_Data &target)
{
    s->prepare();
    return s->read(target.m_Names);
}

bool loadFrom(BinStore *s,ItemPower_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_PowerCategory);
    ok &= s->read(target.m_PowerSet);
    ok &= s->read(target.m_Power);
    ok &= s->read(target.m_Level);
    ok &= s->read(target.m_Remove);
    if(s->isI24Data()) {
        ok &= s->read(target.m_DontSetStance);
        return ok;
    }
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    return s->end_encountered();
}

bool loadFrom(BinStore *s, ShopItemInfo_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_Sell);
    ok &= s->read(target.m_Buy);
    ok &= s->read(target.m_CountPerStore);
    ok &= s->read(target.m_Departments);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    // Only one entry per Item
    if(s->nesting_name(_name))
    {
        s->nest_in();
        if("Power"==_name) {
            ok &= loadFrom(s,target.m_Power);
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}

static bool loadFromI24(BinStore *s, ShopItemInfo_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Name);
    ok &= loadFrom(s,target.m_Power);
    ok &= s->read(target.m_Sell);
    ok &= s->read(target.m_Buy);
    ok &= s->read(target.m_CountPerStore);
    ok &= s->read(target.m_Departments);
    return ok;
}
static bool loadFromI24(BinStore *s, Shop_Data &target)
{
    bool ok = true;
    assert(s->isI24Data());

    s->prepare(); // read the size
    // read fields in order.
    ok &= s->read(target.m_Name);
    ok &= s->handleI24StructArray(target.m_Sells);
    ok &= s->handleI24StructArray(target.m_Buys);
    ok &= s->handleI24StructArray(target.m_Items);
    ok &= s->read(target.m_Salvage);
    ok &= s->read(target.m_Recipe);
    return ok;
}

bool loadFrom(BinStore *s, Shop_Data &target)
{
    bool ok = true;

    assert(!s->isI24Data());

    s->prepare();
    ok &= s->read(target.m_Name);
    if(s->isI24Data()) {
        ok &= s->read(target.m_Salvage);
        ok &= s->read(target.m_Recipe);
    }

    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    // Only one entry per Item
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Sell"==_name)
        {
            ShopBuySell_Data nt;
            ok &= loadFrom(s,nt);
            target.m_Sells.push_back(nt);
        }
        else if("Buy"==_name)
        {
            ShopBuySell_Data nt;
            ok &= loadFrom(s,nt);
            target.m_Buys.push_back(nt);
        }
        else if("Item"==_name)
        {
            ShopItem_Data nt;
            ok &= loadFrom(s,nt);
            target.m_Items.push_back(nt);
        }
        else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}

bool loadFrom(BinStore *s, AllShops_Data &target)
{
    bool ok = true;

    if(s->isI24Data()) {
        s->prepare(); // read the size
        return s->handleI24StructArray(target);
    }

    s->prepare();
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Store"==_name) {
            Shop_Data nt;
            ok &= loadFrom(s,nt);
            target.push_back(nt);
        }
        else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}

bool loadFrom(BinStore *s, AllShopItems_Data &target)
{
    s->prepare(); // read the size
    if(s->isI24Data()) {
        return s->handleI24StructArray(target);
    }
    bool ok = true;
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;

    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Item"==_name)
        {
            ShopItemInfo_Data nt;
            ok &= loadFrom(s,nt);
            target.emplace_back(nt);
        }
        else
            assert(!"unknown field referenced.");

        s->nest_out();
    }

    return ok;
}

bool loadFrom(BinStore * s, AllShopDepts_Data &target)
{
    bool ok = true;
    s->prepare();
    if(s->isI24Data()) {
        return s->handleI24StructArray(target);
    }
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Department"==_name)
        {
            ShopDeptName_Data nt;
            ok &= loadFrom(s,nt);
            target.push_back(nt);
        }
        else
            assert(!"unknown field referenced.");

        s->nest_out();
    }
    return ok;
}

template<class Archive>
static void serialize(Archive & archive, ShopBuySell_Data & m)
{
    archive(cereal::make_nvp("Department",m.m_Department));
    archive(cereal::make_nvp("Markup",m.m_Markup));
}

template<class Archive>
static void serialize(Archive & archive, ShopItem_Data & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
}

template<class Archive>
static void serialize(Archive & archive, Shop_Data & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("Sells",m.m_Sells));
    archive(cereal::make_nvp("Buys",m.m_Buys));
    archive(cereal::make_nvp("Items",m.m_Items));
    archive(cereal::make_nvp("Salvage",m.m_Salvage));
    archive(cereal::make_nvp("Recipe",m.m_Recipe));
}

void saveTo(const AllShops_Data & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"AllShops",baseName,text_format);
}

template<class Archive>
static void serialize(Archive & archive, ItemPower_Data & m)
{
    archive(cereal::make_nvp("PowerCategory",m.m_PowerCategory));
    archive(cereal::make_nvp("PowerSet",m.m_PowerSet));
    archive(cereal::make_nvp("Power",m.m_Power));
    archive(cereal::make_nvp("Level",m.m_Level));
    archive(cereal::make_nvp("Remove",m.m_Remove));
}

template<class Archive>
static void serialize(Archive & archive, ShopItemInfo_Data & m)
{
    archive(cereal::make_nvp("Name",m.m_Name));
    archive(cereal::make_nvp("Sell",m.m_Sell));
    archive(cereal::make_nvp("Buy",m.m_Buy));
    archive(cereal::make_nvp("CountPerStore",m.m_CountPerStore));
    archive(cereal::make_nvp("Departments",m.m_Departments));
    archive(cereal::make_nvp("Power",m.m_Power));
}

void saveTo(const std::vector<struct ShopItemInfo_Data> & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"AllShopItems",baseName,text_format);
}

template<class Archive>
static void serialize(Archive & archive, ShopDeptName_Data & m)
{
    archive(cereal::make_nvp("Name",m.m_Names));
}

void saveTo(const AllShopDepts_Data & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"AllShopDepts",baseName,text_format);
}

//! @}
