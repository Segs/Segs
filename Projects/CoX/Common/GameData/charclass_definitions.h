#pragma once

#include "Common/GameData/attrib_definitions.h"
#include <QString>
#include <vector>
#include <memory>
struct StoredPowerCategory;
struct ClassMod_Data {
    QString Name;
    std::vector<float> Values;
};
struct CharClass_Data
{
    QString m_Name;
    QString m_DisplayName;
    QString m_DisplayHelp;
    QString m_DisplayShortHelp;
    std::vector<Parse_CharAttrib> m_AttribBase;
    std::vector<Parse_CharAttrib> m_AttribMin;
    std::vector<Parse_CharAttrib> m_StrengthMin;
    std::vector<Parse_CharAttrib> m_ResistanceMin;
    std::unique_ptr<Parse_CharAttrib> _FinalAttrMax_;
    std::unique_ptr<Parse_CharAttrib> _FinalAttrMaxMax_;
    std::unique_ptr<Parse_CharAttrib> _FinalAttrStrengthMax_;
    std::unique_ptr<Parse_CharAttrib> _FinalAttrResistanceMax_;
    std::vector<ClassMod_Data> m_ModTable;
    QString m_PrimaryCategory;
    QString m_SecondaryCategory;
    QString m_PowerPoolCategory;
    std::vector<Parse_CharAttribMax> m_AttribMaxTable;
    std::vector<Parse_CharAttribMax> m_AttribMaxMaxTable;
    std::vector<Parse_CharAttribMax> m_StrengthMaxTable;
    std::vector<Parse_CharAttribMax> m_ResistanceMaxTable;
};

struct Parse_AllCharClasses
{
    std::vector<CharClass_Data> m_classes;
};

