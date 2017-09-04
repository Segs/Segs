#include "attrib_serializers.h"
#include "attrib_definitions.h"
#include "serialization_common.h"
#include "DataStorage.h"
namespace {
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
    archive(cereal::make_nvp("Name",m.Name));
    archive(cereal::make_nvp("DisplayName",m.DisplayName));
    archive(cereal::make_nvp("IconName",m.IconName));
}

template<class Archive>
static void serialize(Archive & archive, AttribNames_Data & m)
{
    archive(cereal::make_nvp("Damage",m.m_Damage));
    archive(cereal::make_nvp("Defense",m.m_Defense));
    archive(cereal::make_nvp("Boost",m.m_Boost));
    archive(cereal::make_nvp("Group",m.m_Group));
}

void saveTo(const AttribNames_Data & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"AttributeNames",baseName,text_format);
}

bool loadFrom(BinStore *s, Parse_CharAttrib &target)
{
    s->prepare();

    bool ok = true;
    float *damage_types = &target.m_DamageType00;
    for(int i=0; i<24; ++i) {
        ok &= s->read(damage_types[i]);
    }
    ok &= s->read(target.m_HitPoints);
    ok &= s->read(target.m_Endurance);
    ok &= s->read(target.m_ToHit);
    float *defence_types = &target.m_DefenseType00;
    for(int i=0; i<24; ++i) {
        ok &= s->read(defence_types[i]);
    }

    s->read(target.m_Defense);
    s->read(target.m_Evade);
    s->read(target.m_SpeedRunning);
    s->read(target.m_SpeedFlying);
    s->read(target.m_SpeedSwimming);
    s->read(target.m_SpeedJumping);
    s->read(target.m_JumpHeight);
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
    s->read(target.m_Stunned);
    s->read(target.m_Sleep);
    s->read(target.m_Fly);
    s->read(target.m_Jumppack);
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
    assert(ok & s->end_encountered());
    return ok;
}
bool loadFrom(BinStore *s, Parse_CharAttribMax &target)
{
    s->prepare();

    bool ok = true;
    std::vector<float> *damage_types = &target.m_DamageType00;
    for(int i=0; i<24; ++i) {
        ok &= s->read(damage_types[i]);
    }
    ok &= s->read(target.m_HitPoints);
    ok &= s->read(target.m_Endurance);
    ok &= s->read(target.m_ToHit);
    std::vector<float> *defence_types = &target.m_DefenseType00;
    for(int i=0; i<24; ++i) {
        ok &= s->read(defence_types[i]);
    }

    s->read(target.m_Defense);
    s->read(target.m_Evade);
    s->read(target.m_SpeedRunning);
    s->read(target.m_SpeedFlying);
    s->read(target.m_SpeedSwimming);
    s->read(target.m_SpeedJumping);
    s->read(target.m_JumpHeight);
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
    s->read(target.m_Stunned);
    s->read(target.m_Sleep);
    s->read(target.m_Fly);
    s->read(target.m_Jumppack);
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
    assert(ok & s->end_encountered());
    return ok;
}
