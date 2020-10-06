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

#include "def_serializers.h"

#include "other_definitions.h"
#include "origin_definitions.h"
#include "DataStorage.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"

#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/cereal.hpp>
#include <streambuf>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>
namespace cereal
{
    template<class Archive>
    static void save(Archive & archive, LevelExpAndDebt const & m)
    {
        archive(make_nvp("DefeatPenalty", m.m_DefeatPenalty)
                , make_nvp("ExperienceRequired", m.m_ExperienceRequired) );
    }

    template<class Archive>
    static void load(Archive & archive, LevelExpAndDebt & m)
    {
        archive(make_nvp("DefeatPenalty", m.m_DefeatPenalty)
                , make_nvp("ExperienceRequired", m.m_ExperienceRequired) );
    }

    template<class Archive>
    static void save(Archive & archive, Parse_Combining const & m)
    {
        archive(cereal::make_nvp("CombineChances", m.CombineChances));
    }

    template<class Archive>
    static void load(Archive & archive, Parse_Combining & m)
    {
        archive(cereal::make_nvp("CombineChances", m.CombineChances) );
    }

    template<class Archive>
    static void save(Archive & archive, Parse_Effectiveness const & m)
    {
        archive(cereal::make_nvp("Effectiveness", m.Effectiveness));
    }

    template<class Archive>
    static void load(Archive & archive, Parse_Effectiveness & m)
    {
        archive(cereal::make_nvp("Effectiveness", m.Effectiveness) );
    }

    template<class Archive>
    static void save(Archive & archive, Parse_PI_Schedule const & m)
    {
        archive(cereal::make_nvp("FreeBoostSlotsOnPower", m.m_FreeBoostSlotsOnPower) );
        archive(cereal::make_nvp("PoolPowerSet", m.m_PoolPowerSet) );
        archive(cereal::make_nvp("Power", m.m_Power) );
        archive(cereal::make_nvp("AssignableBoost", m.m_AssignableBoost) );
        archive(cereal::make_nvp("InspirationCol", m.m_InspirationCol) );
        archive(cereal::make_nvp("InspirationRow", m.m_InspirationRow) );
        archive(cereal::make_nvp("BoostSlot", m.m_BoostSlot) );
    }

    template<class Archive>
    static void load(Archive & archive, Parse_PI_Schedule & m)
    {
        archive(cereal::make_nvp("FreeBoostSlotsOnPower", m.m_FreeBoostSlotsOnPower) );
        archive(cereal::make_nvp("PoolPowerSet", m.m_PoolPowerSet) );
        archive(cereal::make_nvp("Power", m.m_Power) );
        archive(cereal::make_nvp("AssignableBoost", m.m_AssignableBoost) );
        archive(cereal::make_nvp("InspirationCol", m.m_InspirationCol) );
        archive(cereal::make_nvp("InspirationRow", m.m_InspirationRow) );
        archive(cereal::make_nvp("BoostSlot", m.m_BoostSlot) );
    }

    template<class Archive>
    void serialize(Archive & archive, Parse_Origin & target)
    {
        archive(cereal::make_nvp("Name",target.Name));
        archive(cereal::make_nvp("DisplayName",target.DisplayName));
        archive(cereal::make_nvp("DisplayHelp",target.DisplayHelp));
        archive(cereal::make_nvp("DisplayShortHelp",target.DisplayShortHelp));
        archive(cereal::make_nvp("NumBonusPowerSets",target.NumBonusPowerSets));
        archive(cereal::make_nvp("NumBonusPowers",target.NumBonusPowers));
        archive(cereal::make_nvp("NumBonusBoostSlots",target.NumBonusBoostSlots));
        archive(cereal::make_nvp("NumContacts",target.NumContacts));
        archive(cereal::make_nvp("ContactBonusLength",target.ContactBonusLength));
    }
} // namespace cereal

//-------------------------------------------------------------------------------------------------------------------
namespace
{
    bool loadFrom(BinStore *s,Parse_Origin *target) {
        s->prepare();
        bool ok = true;
        ok &= s->read(target->Name);
        ok &= s->read(target->DisplayName);
        ok &= s->read(target->DisplayHelp);
        ok &= s->read(target->DisplayShortHelp);
        ok &= s->read(target->NumBonusPowerSets);
        ok &= s->read(target->NumBonusPowers);
        ok &= s->read(target->NumBonusBoostSlots);
        // not a part of schema -> NumBonusInspirationSlots
        ok &= s->read(target->NumContacts);
        ok &= s->read(target->ContactBonusLength);
        ok &= s->prepare_nested(); // will update the file size left
        assert(ok && s->end_encountered());
        return ok;
    }
}

bool loadFrom(BinStore *s, LevelExpAndDebt & target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.m_ExperienceRequired);
    ok &= s->read(target.m_DefeatPenalty);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    return ok;
}

void saveTo(const LevelExpAndDebt & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"LevelExpAndDebt",baseName,text_format);
}

bool loadFrom(BinStore * s, Parse_Combining &target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.CombineChances);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok && s->end_encountered());
    return ok;
}

void saveTo(const Parse_Combining & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"CombiningChances",baseName,text_format);
}

bool loadFrom(BinStore * s, Parse_Effectiveness &target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.Effectiveness);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    return ok;

}

void saveTo(const Parse_Effectiveness & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"BoostEffectiveness",baseName,text_format);

}

bool loadFrom(BinStore * s, Parse_AllOrigins &target)
{
    s->prepare();
    bool ok = true;
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Origin"==_name) {
            target.emplace_back();
            ok &= loadFrom(s,&target.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}

void saveTo(const Parse_AllOrigins & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"Origins",baseName,text_format);
}

bool loadFrom(BinStore * s, Parse_PI_Schedule &target)
{
    s->prepare();
    bool ok = true;
    ok &= s->read(target.m_FreeBoostSlotsOnPower);
    ok &= s->read(target.m_PoolPowerSet);
    ok &= s->read(target.m_Power);
    ok &= s->read(target.m_AssignableBoost);
    ok &= s->read(target.m_InspirationCol);
    ok &= s->read(target.m_InspirationRow);
    ok &= s->read(target.m_BoostSlot);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    return ok;
}

void saveTo(const Parse_PI_Schedule & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"PiSchedule",baseName,text_format);
}

//! @}
