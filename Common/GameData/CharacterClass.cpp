#include "CharacterClass.h"

#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include <cereal/types/memory.hpp>

template<class Archive>
void ClassMod_Data::serialize(Archive &archive)
{
    archive(cereal::make_nvp("Name",Name));
    archive(cereal::make_nvp("Values",Values));
}
SPECIALIZE_SERIALIZATIONS(ClassMod_Data)

template<class Archive>
void CharClass_Data::serialize(Archive & archive)
{
    archive(cereal::make_nvp("Name",m_Name));
    archive(cereal::make_nvp("DisplayName",m_DisplayName));
    archive(cereal::make_nvp("DisplayHelp",m_DisplayHelp));
    archive(cereal::make_nvp("DisplayShortHelp",m_DisplayShortHelp));
    archive(cereal::make_nvp("PrimaryCategory",m_PrimaryCategory));
    archive(cereal::make_nvp("SecondaryCategory",m_SecondaryCategory));
    archive(cereal::make_nvp("PowerPoolCategory",m_PowerPoolCategory));
    archive(cereal::make_nvp("AttribMin",m_AttribMin));
    archive(cereal::make_nvp("AttribBase",m_AttribBase));
    archive(cereal::make_nvp("StrengthMin",m_StrengthMin));
    archive(cereal::make_nvp("ResistanceMin",m_ResistanceMin));
    archive(cereal::make_nvp("AttribMaxTable",m_AttribMaxTable));
    archive(cereal::make_nvp("AttribMaxMaxTable",m_AttribMaxMaxTable));
    archive(cereal::make_nvp("StrengthMaxTable",m_StrengthMaxTable));
    archive(cereal::make_nvp("ResistanceMaxTable",m_ResistanceMaxTable));
    archive(cereal::make_nvp("ModTable",m_ModTable));
    archive(cereal::make_nvp("_FinalAttrMax_",_FinalAttrMax_));
    archive(cereal::make_nvp("_FinalAttrMaxMax_",_FinalAttrMaxMax_));
    archive(cereal::make_nvp("_FinalAttrStrengthMax_",_FinalAttrStrengthMax_));
    archive(cereal::make_nvp("_FinalAttrResistanceMax_",_FinalAttrResistanceMax_));

}
SPECIALIZE_SERIALIZATIONS(CharClass_Data)

