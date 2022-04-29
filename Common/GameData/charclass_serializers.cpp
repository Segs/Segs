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

#include "GameData/CharacterClass.h"
#include "GameData/CharacterAttributes.h"
#include "charclass_serializers.h"
#include "attrib_serializers.h"

#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include <cereal/types/memory.hpp>

#include "DataStorage.h"
namespace
{
    bool loadFrom(BinStore *s, ClassMod_Data &target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target.Name);
        ok &= s->read(target.Values);
        ok &= s->prepare_nested(); // will update the file size left
        assert(ok && s->end_encountered());
        return ok;
    }

    bool loadFrom(BinStore *s, CharClass_Data &target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target.m_Name);
        ok &= s->read(target.m_DisplayName);
        ok &= s->read(target.m_DisplayHelp);
        ok &= s->read(target.m_DisplayShortHelp);
        ok &= s->read(target.m_PrimaryCategory);
        ok &= s->read(target.m_SecondaryCategory);
        ok &= s->read(target.m_PowerPoolCategory);
        ok &= s->prepare_nested(); // will update the file size left
        if(s->end_encountered())
            return ok;
        QByteArray _name;
        while(s->nesting_name(_name))
        {
            s->nest_in();
            if("AttribMin"==_name) {
                target.m_AttribMin.emplace_back();
                ok &= loadFrom(s,target.m_AttribMin.back());
            } else if("AttribBase"==_name) {
                target.m_AttribBase.emplace_back();
                ok &= loadFrom(s,target.m_AttribBase.back());
            } else if("StrengthMin"==_name) {
                target.m_StrengthMin.emplace_back();
                ok &= loadFrom(s,target.m_StrengthMin.back());
            } else if("ResistanceMin"==_name) {
                target.m_ResistanceMin.emplace_back();
                ok &= loadFrom(s,target.m_ResistanceMin.back());
            } else if("AttribMaxTable"==_name) {
                target.m_AttribMaxTable.emplace_back();
                ok &= loadFrom(s,target.m_AttribMaxTable.back());
            } else if("AttribMaxMaxTable"==_name) {
                target.m_AttribMaxMaxTable.emplace_back();
                ok &= loadFrom(s,target.m_AttribMaxMaxTable.back());
            } else if("StrengthMaxTable"==_name) {
                target.m_StrengthMaxTable.emplace_back();
                ok &= loadFrom(s,target.m_StrengthMaxTable.back());
            } else if("ResistanceMaxTable"==_name) {
                target.m_ResistanceMaxTable.emplace_back();
                ok &= loadFrom(s,target.m_ResistanceMaxTable.back());
            } else if("ModTable"==_name) {
                target.m_ModTable.emplace_back();
                ok &= loadFrom(s,target.m_ModTable.back());
            } else if("_FinalAttrMax_"==_name) {
                target._FinalAttrMax_.reset(new Parse_CharAttrib);
                ok &= loadFrom(s,*target._FinalAttrMax_);
            } else if("_FinalAttrMaxMax_"==_name) {
                target._FinalAttrMaxMax_.reset(new Parse_CharAttrib);
                ok &= loadFrom(s,*target._FinalAttrMaxMax_);
            } else if("_FinalAttrStrengthMax_"==_name) {
                target._FinalAttrStrengthMax_.reset(new Parse_CharAttrib);
                ok &= loadFrom(s,*target._FinalAttrStrengthMax_);
            } else if("_FinalAttrResistanceMax_"==_name) {
                target._FinalAttrResistanceMax_.reset(new Parse_CharAttrib);
                ok &= loadFrom(s,*target._FinalAttrResistanceMax_);
            } else
                assert(!"unknown field referenced.");
            s->nest_out();
        }
        assert(ok);
        return ok;
    }
} // namespace

static bool loadFromI24(BinStore *s, ClassMod_Data &target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.Name);
    ok &= s->read(target.Values);
    return ok;
}
static bool loadFromI24(BinStore *s,CharClass_Data &target) {
    bool ok = true;
    s->prepare();
    ok &= s->read(target.m_Name);
    ok &= s->read(target.m_DisplayName);
    ok &= s->read(target.m_DisplayHelp);
    ok &= s->read(target.m_AllowedOrigins);
    ok &= s->read(target.m_SpecialRestrictions);
    ok &= s->read(target.m_StoreRequires);
    ok &= s->read(target.m_LockedTooltip);
    ok &= s->read(target.m_ProductCode);
    ok &= s->read(target.m_ReductionClass);
    ok &= s->read(target.m_ReduceAsArchvillain);
    ok &= s->read(target.m_LevelUpRespecs);
    ok &= s->read(target.m_DisplayShortHelp);
    ok &= s->read(target.m_Icon);
    ok &= s->read(target.m_PrimaryCategory);
    ok &= s->read(target.m_SecondaryCategory);
    ok &= s->read(target.m_PowerPoolCategory);
    ok &= s->read(target.m_EpicPoolCategory);
    ok &= s->handleI24StructArray(target.m_AttribMin);
    ok &= s->handleI24StructArray(target.m_AttribBase);
    ok &= s->handleI24StructArray(target.m_StrengthMin);
    ok &= s->handleI24StructArray(target.m_ResistanceMin);
    ok &= s->handleI24StructArray(target.m_AtrribDiminStrIn);
    ok &= s->handleI24StructArray(target.m_AtrribDiminStrOut);
    ok &= s->handleI24StructArray(target.m_AtrribDiminCurIn);
    ok &= s->handleI24StructArray(target.m_AtrribDiminCurOut);
    ok &= s->handleI24StructArray(target.m_AtrribDiminResIn);
    ok &= s->handleI24StructArray(target.m_AtrribDiminResOut);
    ok &= s->handleI24StructArray(target.m_AttribMaxTable);
    ok &= s->handleI24StructArray(target.m_AttribMaxMaxTable);
    ok &= s->handleI24StructArray(target.m_StrengthMaxTable);
    ok &= s->handleI24StructArray(target.m_ResistanceMaxTable);
    ok &= s->handleI24StructArray(target.m_ModTable);
    ok &= s->read(target.m_ConnectHPAndStatus);
    ok &= s->read(target.m_DefiantHitPointsAttrib);
    ok &= s->read(target.m_DefiantScale);
    return ok;
}
bool loadFrom(BinStore *s, Parse_AllCharClasses &target)
{
    s->prepare();
    if(s->isI24Data()) {
        return s->handleI24StructArray(target);
    }
    bool ok = s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Class"==_name) {
            target.emplace_back();
            ok &= loadFrom(s,target.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

void saveTo(const Parse_AllCharClasses & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"EntityClasses",baseName,text_format);
}

//! @}
