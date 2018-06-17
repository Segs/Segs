/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <QtCore/QString>
#include <vector>

struct ItemPower_Data
{
    QString m_PowerCategory;
    QString m_PowerSet;
    QString m_Power;
    int m_Level;
    int m_Remove;
};

struct ShopItemInfo_Data
{
    QString m_Name;
    ItemPower_Data m_Power;
    int m_Sell;
    int m_Buy;
    int m_CountPerStore;
    std::vector<uint32_t> m_Departments;
    struct Power_Data *power_tpl; // looked up by using entries in m_Power
};
typedef std::vector<ShopItemInfo_Data> AllShopItems_Data;

struct ShopBuySell_Data
{
    int m_Department;
    float m_Markup;
};

struct ShopItem_Data
{
    QString m_Name;
};

struct Shop_Data
{
    QString m_Name;
    std::vector<ShopBuySell_Data> m_Sells;
    std::vector<ShopBuySell_Data> m_Buys;
    std::vector<ShopItem_Data> m_Items;
};
typedef std::vector<Shop_Data> AllShops_Data;

struct ShopDeptName_Data
{
    QString m_Names;
};
typedef std::vector<ShopDeptName_Data> AllShopDepts_Data;
