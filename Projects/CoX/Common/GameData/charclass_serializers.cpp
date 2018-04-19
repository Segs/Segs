#include "charclass_serializers.h"
#include "charclass_definitions.h"
#include "attrib_serializers.h"
#include "attrib_definitions.h"

#include "serialization_common.h"
#include <cereal/types/memory.hpp>

#include "DataStorage.h"
namespace {
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
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("AttribMin")==0) {
            target.m_AttribMin.emplace_back();
            ok &= loadFrom(s,target.m_AttribMin.back());
        } else if(_name.compare("AttribBase")==0) {
            target.m_AttribBase.emplace_back();
            ok &= loadFrom(s,target.m_AttribBase.back());
        } else if(_name.compare("StrengthMin")==0) {
            target.m_StrengthMin.emplace_back();
            ok &= loadFrom(s,target.m_StrengthMin.back());
        } else if(_name.compare("ResistanceMin")==0) {
            target.m_ResistanceMin.emplace_back();
            ok &= loadFrom(s,target.m_ResistanceMin.back());
        } else if(_name.compare("AttribMaxTable")==0) {
            target.m_AttribMaxTable.emplace_back();
            ok &= loadFrom(s,target.m_AttribMaxTable.back());
        } else if(_name.compare("AttribMaxMaxTable")==0) {
            target.m_AttribMaxMaxTable.emplace_back();
            ok &= loadFrom(s,target.m_AttribMaxMaxTable.back());
        } else if(_name.compare("StrengthMaxTable")==0) {
            target.m_StrengthMaxTable.emplace_back();
            ok &= loadFrom(s,target.m_StrengthMaxTable.back());
        } else if(_name.compare("ResistanceMaxTable")==0) {
            target.m_ResistanceMaxTable.emplace_back();
            ok &= loadFrom(s,target.m_ResistanceMaxTable.back());
        } else if(_name.compare("ModTable")==0) {
            target.m_ModTable.emplace_back();
            ok &= loadFrom(s,target.m_ModTable.back());
        } else if(_name.compare("_FinalAttrMax_")==0) {
            target._FinalAttrMax_.reset(new Parse_CharAttrib);
            ok &= loadFrom(s,*target._FinalAttrMax_);
        } else if(_name.compare("_FinalAttrMaxMax_")==0) {
            target._FinalAttrMaxMax_.reset(new Parse_CharAttrib);
            ok &= loadFrom(s,*target._FinalAttrMaxMax_);
        } else if(_name.compare("_FinalAttrStrengthMax_")==0) {
            target._FinalAttrStrengthMax_.reset(new Parse_CharAttrib);
            ok &= loadFrom(s,*target._FinalAttrStrengthMax_);
        } else if(_name.compare("_FinalAttrResistanceMax_")==0) {
            target._FinalAttrResistanceMax_.reset(new Parse_CharAttrib);
            ok &= loadFrom(s,*target._FinalAttrResistanceMax_);
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

}
bool loadFrom(BinStore *s, Parse_AllCharClasses &target)
{
    s->prepare();
    bool ok = s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Class")==0) {
            target.emplace_back();
            ok &= loadFrom(s,target.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}
template<class Archive>
static void serialize(Archive &archive, ClassMod_Data &src)
{
    archive(cereal::make_nvp("Name",src.Name));
    archive(cereal::make_nvp("Values",src.Values));
}
template<class Archive>
static void serialize(Archive & archive, CharClass_Data & src)
{
    archive(cereal::make_nvp("Name",src.m_Name));
    archive(cereal::make_nvp("DisplayName",src.m_DisplayName));
    archive(cereal::make_nvp("DisplayHelp",src.m_DisplayHelp));
    archive(cereal::make_nvp("DisplayShortHelp",src.m_DisplayShortHelp));
    archive(cereal::make_nvp("PrimaryCategory",src.m_PrimaryCategory));
    archive(cereal::make_nvp("SecondaryCategory",src.m_SecondaryCategory));
    archive(cereal::make_nvp("PowerPoolCategory",src.m_PowerPoolCategory));
    archive(cereal::make_nvp("AttribMin",src.m_AttribMin));
    archive(cereal::make_nvp("AttribBase",src.m_AttribBase));
    archive(cereal::make_nvp("StrengthMin",src.m_StrengthMin));
    archive(cereal::make_nvp("ResistanceMin",src.m_ResistanceMin));
    archive(cereal::make_nvp("AttribMaxTable",src.m_AttribMaxTable));
    archive(cereal::make_nvp("AttribMaxMaxTable",src.m_AttribMaxMaxTable));
    archive(cereal::make_nvp("StrengthMaxTable",src.m_StrengthMaxTable));
    archive(cereal::make_nvp("ResistanceMaxTable",src.m_ResistanceMaxTable));
    archive(cereal::make_nvp("ModTable",src.m_ModTable));
    archive(cereal::make_nvp("_FinalAttrMax_",src._FinalAttrMax_));
    archive(cereal::make_nvp("_FinalAttrMaxMax_",src._FinalAttrMaxMax_));
    archive(cereal::make_nvp("_FinalAttrStrengthMax_",src._FinalAttrStrengthMax_));
    archive(cereal::make_nvp("_FinalAttrResistanceMax_",src._FinalAttrResistanceMax_));

}
void saveTo(const Parse_AllCharClasses & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"EntityClasses",baseName,text_format);
}
