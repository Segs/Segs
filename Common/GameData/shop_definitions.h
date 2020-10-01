/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <QtCore/QString>
#include <vector>

struct ItemPower_Data
{
    QByteArray m_PowerCategory;
    QByteArray m_PowerSet;
    QByteArray m_Power;
    int m_Level;
    int m_Remove;
};

struct ShopItemInfo_Data
{
    QByteArray m_Name;
    ItemPower_Data m_Power;
    int m_Sell;
    int m_Buy;
    int m_CountPerStore;
    std::vector<uint32_t> m_Departments;
    struct Power_Data *power_tpl; // looked up by using entries in m_Power
};
using AllShopItems_Data = std::vector<ShopItemInfo_Data>;

struct ShopBuySell_Data
{
    int m_Department;
    float m_Markup;
};

struct ShopItem_Data
{
    QByteArray m_Name;
};

struct Shop_Data
{
    QByteArray m_Name;
    std::vector<ShopBuySell_Data> m_Sells;
    std::vector<ShopBuySell_Data> m_Buys;
    std::vector<ShopItem_Data> m_Items;
};
using AllShops_Data = std::vector<Shop_Data>;

struct ShopDeptName_Data
{
    QByteArray m_Names;
};
using AllShopDepts_Data = std::vector<ShopDeptName_Data>;
