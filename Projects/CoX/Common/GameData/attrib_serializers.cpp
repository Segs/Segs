/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "attrib_serializers.h"
#include "attrib_definitions.h"
#include "serialization_common.h"
#include "DataStorage.h"

namespace
{
    bool loadFrom(BinStore * s, Parse_AttribDesc & target)
    {
        s->prepare();
        bool ok = true;
        ok &= s->read(target.Name);
        ok &= s->read(target.DisplayName);
        ok &= s->read(target.IconName);
        ok &= s->prepare_nested(); // will update the file size left
        assert(s->end_encountered());
        return ok;
    }
}

bool loadFrom(BinStore * s, AttribNames_Data & target)
{
    s->prepare();
    bool ok = s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Damage")==0) {
            target.m_Damage.emplace_back();
            ok &= loadFrom(s,target.m_Damage.back());
        } else if(_name.compare("Defense")==0) {
            target.m_Defense.emplace_back();
            ok &= loadFrom(s,target.m_Defense.back());
        } else if(_name.compare("Boost")==0) {
            target.m_Boost.emplace_back();
            ok &= loadFrom(s,target.m_Boost.back());
        } else if(_name.compare("Group")==0) {
            target.m_Group.emplace_back();
            ok &= loadFrom(s,target.m_Group.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

template<class Archive>
static void serialize(Archive & archive, Parse_AttribDesc & m)
{
    try
    {
        archive(cereal::make_nvp("Name",m.Name));
        archive(cereal::make_nvp("DisplayName",m.DisplayName));
        archive(cereal::make_nvp("IconName",m.IconName));
    }
    catch(cereal::RapidJSONException &e)
    {
        qWarning() << e.what();
    }
    catch(std::exception &e)
    {
        qCritical() << "Other kind of exception: " << e.what();
    }

}

template<class Archive>
static void serialize(Archive & archive, AttribNames_Data & m)
{
    try
    {
        archive(cereal::make_nvp("Damage",m.m_Damage));
        archive(cereal::make_nvp("Defense",m.m_Defense));
        archive(cereal::make_nvp("Boost",m.m_Boost));
        archive(cereal::make_nvp("Group",m.m_Group));
    }
    catch(cereal::RapidJSONException &e)
    {
        qWarning() << e.what();
    }
    catch(std::exception &e)
    {
        qCritical() << "Other kind of exception: " << e.what();
    }
}

template<class Archive>
void serialize(Archive & archive, Parse_CharAttrib & target)
{
    try
    {
        char buf[128];
        for(int i=0; i<24; ++i)
        {
            sprintf(buf,"DamageType%02d",i);
            archive(cereal::make_nvp(buf,target.m_DamageTypes[i]));
        }
        archive(cereal::make_nvp("HitPoints",target.m_HitPoints));
        archive(cereal::make_nvp("Endurance",target.m_Endurance));
        archive(cereal::make_nvp("ToHit",target.m_ToHit));
        for(int i=0; i<24; ++i) {
            sprintf(buf,"DefenseType00%02d",i);
            archive(cereal::make_nvp(buf,target.m_DefenseTypes[i]));
        }

        archive(cereal::make_nvp("Defense",target.m_Defense));
        archive(cereal::make_nvp("Evade",target.m_Evade));
        archive(cereal::make_nvp("SpeedRunning",target.m_SpeedRunning));
        archive(cereal::make_nvp("SpeedFlying",target.m_SpeedFlying));
        archive(cereal::make_nvp("SpeedSwimming",target.m_SpeedSwimming));
        archive(cereal::make_nvp("SpeedJumping",target.m_SpeedJumping));
        archive(cereal::make_nvp("JumpHeight",target.m_jump_height));
        archive(cereal::make_nvp("MovementControl",target.m_MovementControl));
        archive(cereal::make_nvp("MovementFriction",target.m_MovementFriction));
        archive(cereal::make_nvp("Stealth",target.m_Stealth));
        archive(cereal::make_nvp("StealthRadius",target.m_StealthRadius));
        archive(cereal::make_nvp("PerceptionRadius",target.m_PerceptionRadius));
        archive(cereal::make_nvp("Regeneration",target.m_Regeneration));
        archive(cereal::make_nvp("Recovery",target.m_Recovery));
        archive(cereal::make_nvp("ThreatLevel",target.m_ThreatLevel));
        archive(cereal::make_nvp("Taunt",target.m_Taunt));
        archive(cereal::make_nvp("Confused",target.m_Confused));
        archive(cereal::make_nvp("Afraid",target.m_Afraid));
        archive(cereal::make_nvp("Held",target.m_Held));
        archive(cereal::make_nvp("Immobilized",target.m_Immobilized));
        archive(cereal::make_nvp("Stunned",target.m_is_stunned));
        archive(cereal::make_nvp("Sleep",target.m_Sleep));
        archive(cereal::make_nvp("Fly",target.m_is_flying));
        archive(cereal::make_nvp("Jumppack",target.m_has_jumppack));
        archive(cereal::make_nvp("Teleport",target.m_Teleport));
        archive(cereal::make_nvp("Untouchable",target.m_Untouchable));
        archive(cereal::make_nvp("Intangible",target.m_Intangible));
        archive(cereal::make_nvp("OnlyAffectsSelf",target.m_OnlyAffectsSelf));
        archive(cereal::make_nvp("Knockup",target.m_Knockup));
        archive(cereal::make_nvp("Knockback",target.m_Knockback));
        archive(cereal::make_nvp("Repel",target.m_Repel));
        archive(cereal::make_nvp("Accuracy",target.m_Accuracy));
        archive(cereal::make_nvp("Radius",target.m_Radius));
        archive(cereal::make_nvp("Arc",target.m_Arc));
        archive(cereal::make_nvp("Range",target.m_Range));
        archive(cereal::make_nvp("TimeToActivate",target.m_TimeToActivate));
        archive(cereal::make_nvp("RechargeTime",target.m_RechargeTime));
        archive(cereal::make_nvp("InterruptTime",target.m_InterruptTime));
        archive(cereal::make_nvp("EnduranceDiscount",target.m_EnduranceDiscount));
    }
    catch(cereal::RapidJSONException &e)
    {
        qWarning() << e.what();
    }
    catch(std::exception &e)
    {
        qCritical() << "Other kind of exception: " << e.what();
    }
}

template<class Archive>
void serialize(Archive & archive, Parse_CharAttribMax & target)
{
    try
    {
        char buf[128];
        for(int i=0; i<24; ++i)
        {
            sprintf(buf,"DamageType%02d",i);
            archive(cereal::make_nvp(buf,target.m_DamageTypes[i]));
        }
        archive(cereal::make_nvp("HitPoints",target.m_HitPoints));
        archive(cereal::make_nvp("Endurance",target.m_Endurance));
        archive(cereal::make_nvp("ToHit",target.m_ToHit));
        for(int i=0; i<24; ++i) {
            sprintf(buf,"DefenseType00%02d",i);
            archive(cereal::make_nvp(buf,target.m_DefenseTypes[i]));
        }

        archive(cereal::make_nvp("Defense",target.m_Defense));
        archive(cereal::make_nvp("Evade",target.m_Evade));
        archive(cereal::make_nvp("SpeedRunning",target.m_SpeedRunning));
        archive(cereal::make_nvp("SpeedFlying",target.m_SpeedFlying));
        archive(cereal::make_nvp("SpeedSwimming",target.m_SpeedSwimming));
        archive(cereal::make_nvp("SpeedJumping",target.m_SpeedJumping));
        archive(cereal::make_nvp("JumpHeight",target.m_jump_height));
        archive(cereal::make_nvp("MovementControl",target.m_MovementControl));
        archive(cereal::make_nvp("MovementFriction",target.m_MovementFriction));
        archive(cereal::make_nvp("Stealth",target.m_Stealth));
        archive(cereal::make_nvp("StealthRadius",target.m_StealthRadius));
        archive(cereal::make_nvp("PerceptionRadius",target.m_PerceptionRadius));
        archive(cereal::make_nvp("Regeneration",target.m_Regeneration));
        archive(cereal::make_nvp("Recovery",target.m_Recovery));
        archive(cereal::make_nvp("ThreatLevel",target.m_ThreatLevel));
        archive(cereal::make_nvp("Taunt",target.m_Taunt));
        archive(cereal::make_nvp("Confused",target.m_Confused));
        archive(cereal::make_nvp("Afraid",target.m_Afraid));
        archive(cereal::make_nvp("Held",target.m_Held));
        archive(cereal::make_nvp("Immobilized",target.m_Immobilized));
        archive(cereal::make_nvp("Stunned",target.m_is_stunned));
        archive(cereal::make_nvp("Sleep",target.m_Sleep));
        archive(cereal::make_nvp("Fly",target.m_is_flying));
        archive(cereal::make_nvp("Jumppack",target.m_has_jumppack));
        archive(cereal::make_nvp("Teleport",target.m_Teleport));
        archive(cereal::make_nvp("Untouchable",target.m_Untouchable));
        archive(cereal::make_nvp("Intangible",target.m_Intangible));
        archive(cereal::make_nvp("OnlyAffectsSelf",target.m_OnlyAffectsSelf));
        archive(cereal::make_nvp("Knockup",target.m_Knockup));
        archive(cereal::make_nvp("Knockback",target.m_Knockback));
        archive(cereal::make_nvp("Repel",target.m_Repel));
        archive(cereal::make_nvp("Accuracy",target.m_Accuracy));
        archive(cereal::make_nvp("Radius",target.m_Radius));
        archive(cereal::make_nvp("Arc",target.m_Arc));
        archive(cereal::make_nvp("Range",target.m_Range));
        archive(cereal::make_nvp("TimeToActivate",target.m_TimeToActivate));
        archive(cereal::make_nvp("RechargeTime",target.m_RechargeTime));
        archive(cereal::make_nvp("InterruptTime",target.m_InterruptTime));
        archive(cereal::make_nvp("EnduranceDiscount",target.m_EnduranceDiscount));
    }
    catch(cereal::RapidJSONException &e)
    {
        qWarning() << e.what();
    }
    catch(std::exception &e)
    {
        qCritical() << "Other kind of exception: " << e.what();
    }

}

void saveTo(const AttribNames_Data & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"AttributeNames",baseName,text_format);
}

bool loadFrom(BinStore *s, Parse_CharAttrib &target)
{
    s->prepare();

    bool ok = true;
    for(int i=0; i<24; ++i) {
        ok &= s->read(target.m_DamageTypes[i]);
    }
    ok &= s->read(target.m_HitPoints);
    ok &= s->read(target.m_Endurance);
    ok &= s->read(target.m_ToHit);
    for(int i=0; i<24; ++i) {
        ok &= s->read(target.m_DefenseTypes[i]);
    }

    s->read(target.m_Defense);
    s->read(target.m_Evade);
    s->read(target.m_SpeedRunning);
    s->read(target.m_SpeedFlying);
    s->read(target.m_SpeedSwimming);
    s->read(target.m_SpeedJumping);
    s->read(target.m_jump_height);
    s->read(target.m_MovementControl);
    s->read(target.m_MovementFriction);
    s->read(target.m_Stealth);
    s->read(target.m_StealthRadius);
    s->read(target.m_PerceptionRadius);
    s->read(target.m_Regeneration);
    s->read(target.m_Recovery);
    s->read(target.m_ThreatLevel);
    s->read(target.m_Taunt);
    s->read(target.m_Confused);
    s->read(target.m_Afraid);
    s->read(target.m_Held);
    s->read(target.m_Immobilized);
    s->read(target.m_is_stunned);
    s->read(target.m_Sleep);
    s->read(target.m_is_flying);
    s->read(target.m_has_jumppack);
    s->read(target.m_Teleport);
    s->read(target.m_Untouchable);
    s->read(target.m_Intangible);
    s->read(target.m_OnlyAffectsSelf);
    s->read(target.m_Knockup);
    s->read(target.m_Knockback);
    s->read(target.m_Repel);
    s->read(target.m_Accuracy);
    s->read(target.m_Radius);
    s->read(target.m_Arc);
    s->read(target.m_Range);
    s->read(target.m_TimeToActivate);
    s->read(target.m_RechargeTime);
    s->read(target.m_InterruptTime);
    s->read(target.m_EnduranceDiscount);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok && s->end_encountered());
    return ok;
}

bool loadFrom(BinStore *s, Parse_CharAttribMax &target)
{
    s->prepare();

    bool ok = true;
    for(int i=0; i<24; ++i) {
        ok &= s->read(target.m_DamageTypes[i]);
    }
    ok &= s->read(target.m_HitPoints);
    ok &= s->read(target.m_Endurance);
    ok &= s->read(target.m_ToHit);
    for(int i=0; i<24; ++i) {
        ok &= s->read(target.m_DefenseTypes[i]);
    }

    s->read(target.m_Defense);
    s->read(target.m_Evade);
    s->read(target.m_SpeedRunning);
    s->read(target.m_SpeedFlying);
    s->read(target.m_SpeedSwimming);
    s->read(target.m_SpeedJumping);
    s->read(target.m_jump_height);
    s->read(target.m_MovementControl);
    s->read(target.m_MovementFriction);
    s->read(target.m_Stealth);
    s->read(target.m_StealthRadius);
    s->read(target.m_PerceptionRadius);
    s->read(target.m_Regeneration);
    s->read(target.m_Recovery);
    s->read(target.m_ThreatLevel);
    s->read(target.m_Taunt);
    s->read(target.m_Confused);
    s->read(target.m_Afraid);
    s->read(target.m_Held);
    s->read(target.m_Immobilized);
    s->read(target.m_is_stunned);
    s->read(target.m_Sleep);
    s->read(target.m_is_flying);
    s->read(target.m_has_jumppack);
    s->read(target.m_Teleport);
    s->read(target.m_Untouchable);
    s->read(target.m_Intangible);
    s->read(target.m_OnlyAffectsSelf);
    s->read(target.m_Knockup);
    s->read(target.m_Knockback);
    s->read(target.m_Repel);
    s->read(target.m_Accuracy);
    s->read(target.m_Radius);
    s->read(target.m_Arc);
    s->read(target.m_Range);
    s->read(target.m_TimeToActivate);
    s->read(target.m_RechargeTime);
    s->read(target.m_InterruptTime);
    s->read(target.m_EnduranceDiscount);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok && s->end_encountered());
    return ok;
}

//template instantiations
template
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive, Parse_CharAttrib & m);
template
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive, Parse_CharAttrib & m);
template
void serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive & archive, Parse_CharAttrib & m);
template
void serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive & archive, Parse_CharAttrib & m);

template
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive, Parse_CharAttribMax & m);
template
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive, Parse_CharAttribMax & m);
template
void serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive & archive, Parse_CharAttribMax & m);
template
void serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive & archive, Parse_CharAttribMax & m);

void serializeToDb(const Parse_CharAttrib &data, QString &tgt)
{
    std::ostringstream ostr;
    {
        cereal::JSONOutputArchive ar(ostr);
        ar(data);
    }
    tgt = QString::fromStdString(ostr.str());
}

void serializeFromDb(Parse_CharAttrib &data,const QString &src)
{
    if(src.isEmpty())
        return;
    std::istringstream istr;
    istr.str(src.toStdString());
    {
        cereal::JSONInputArchive ar(istr);
        ar(data);
    }
}

//! @}
