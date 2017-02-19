#include "shop_serializers.h"
#include "serialization_common.h"

#include "Common/GameData/shop_definitions.h"
#include "DataStorage.h"

//static Serialization_Template ShopItem_Tokens[9] = {
//    { "{", TOKEN_START},
//    { "Name", TOKEN_STRING_LIST, 0},
//    { "Power", TOKEN_SUB_TABLE_400, 4, 0x14, PowerTokens},
//    { "Sell", TOKEN_ENUM, 0x18},
//    { "Buy", TOKEN_ENUM, 0x1C},
//    { "CountPerStore", TOKEN_ENUM, 0x20},
//    { "Departments", TOKEN_INT_VECTOR, 0x24},
//    { "}", TOKEN_END},
//    { ""}
//};
//static Serialization_Template StoreItems_Tokens[2] = {
//    { "Item", TOKEN_SUB_TABLE, 0, 44, ShopItem_Tokens },
//    { ""}
//};
//static Serialization_Template DeptName_Tokens[3] =
//{
//    { "Name", TOKEN_STRING_LIST, 4},
//    { "\n", TOKEN_END},
//    { }
//};
//static Serialization_Template Depts_Tokens[2] =
//{
//    { "Department", TOKEN_SUB_TABLE, 0, 8, DeptName_Tokens },
//    { }
//};
//static Serialization_Template BuySell_Tokens[4] = {
//    { "Department", TOKEN_VAL_LIST, 0},
//    { "Markup", TOKEN_VAL2_LIST, 4},
//    { "\n", TOKEN_END},
//    { }
//};
//static Serialization_Template ItemName_Tokens[3] = {
//    { "Name", TOKEN_STRING_LIST, 0},
//    { "\n", TOKEN_END},
//    { },
//};
//static Serialization_Template Store_Tokens[7] = {
//    { "{", TOKEN_START},
//    { "Name", TOKEN_STRING_LIST, 0},
//    { "Sell", TOKEN_SUB_TABLE, 4, 8, BuySell_Tokens},
//    { "Buy", TOKEN_SUB_TABLE, 8, 8, BuySell_Tokens},
//    { "Item", TOKEN_SUB_TABLE, 0xC, 4, ItemName_Tokens},
//    { "}", TOKEN_END},
//    { },
//};
//static Serialization_Template AllStores_Tokens[2] = {
//    { "Store", TOKEN_SUB_TABLE, 0, 16, Store_Tokens },
//    { }
//};

namespace {

bool loadFrom(BinStore *s,ShopBuySell_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Department);
    ok &= s->read(target.m_Markup);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    return(s->end_encountered());
}
bool loadFrom(BinStore *s,ShopItem_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Name);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    return(s->end_encountered());
}
bool loadFrom(BinStore *s,ShopDeptName_Data *target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target->m_Names);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    return(s->end_encountered());
}
bool loadFrom(BinStore *s,Shop_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Name);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QString _name;
    // Only one entry per Item
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Sell")==0) {
            ShopBuySell_Data nt;
            ok &= loadFrom(s,nt);
            target.m_Sells.push_back(nt);
        } else if(_name.compare("Buy")==0) {
            ShopBuySell_Data nt;
            ok &= loadFrom(s,nt);
            target.m_Buys.push_back(nt);
        } else if(_name.compare("Item")==0) {
            ShopItem_Data nt;
            ok &= loadFrom(s,nt);
            target.m_Items.push_back(nt);
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}
bool loadFrom(BinStore *s,ItemPower_Data *target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target->m_PowerCategory);
    ok &= s->read(target->m_PowerSet);
    ok &= s->read(target->m_Power);
    ok &= s->read(target->m_Level);
    ok &= s->read(target->m_Remove);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    return (s->end_encountered());
}
bool loadFrom(BinStore *s,ShopItemInfo_Data *target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target->m_Name);
    ok &= s->read(target->m_Sell);
    ok &= s->read(target->m_Buy);
    ok &= s->read(target->m_CountPerStore);
    ok &= s->read(target->m_Departments);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QString _name;
    // Only one entry per Item
    if(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Power")==0) {
            ok &= loadFrom(s,&target->m_Power);
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}
}

bool loadFrom(BinStore *s, AllShops_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Store")==0) {
            Shop_Data nt;
            ok &= loadFrom(s,nt);
            target.push_back(nt);
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}

bool loadFrom(BinStore * s, AllShopItems_Data *target)
{
    bool ok = true;
    s->prepare();
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Item")==0) {
            ShopItemInfo_Data nt;
            ok &= loadFrom(s,&nt);
            target->emplace_back(nt);
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}

bool loadFrom(BinStore * s, AllShopDepts_Data *target)
{
    bool ok = true;
    s->prepare();
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Department")==0) {
            ShopDeptName_Data nt;
            ok &= loadFrom(s,&nt);
            target->push_back(nt);
        } else
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

void saveTo(const AllShopItems_Data & target, const QString & baseName, bool text_format)
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
