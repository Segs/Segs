/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CharacterAttributes.h"

#include <QString>
#include <vector>
#include <memory>

struct ClassMod_Data
{
    QByteArray Name;
    std::vector<float> Values;
    template<class Archive>
    void serialize(Archive & archive);
};

struct CharClass_Data
{
    QByteArray m_Name;
    QByteArray m_DisplayName;
    QByteArray m_DisplayHelp;
    QByteArray m_DisplayShortHelp;
    std::vector<Parse_CharAttrib> m_AttribBase;
    std::vector<Parse_CharAttrib> m_AttribMin;
    std::vector<Parse_CharAttrib> m_StrengthMin;
    std::vector<Parse_CharAttrib> m_ResistanceMin;
    std::unique_ptr<Parse_CharAttrib> _FinalAttrMax_;
    std::unique_ptr<Parse_CharAttrib> _FinalAttrMaxMax_;
    std::unique_ptr<Parse_CharAttrib> _FinalAttrStrengthMax_;
    std::unique_ptr<Parse_CharAttrib> _FinalAttrResistanceMax_;
    std::vector<ClassMod_Data> m_ModTable;
    QByteArray m_PrimaryCategory;
    QByteArray m_SecondaryCategory;
    QByteArray m_PowerPoolCategory;
    std::vector<Parse_CharAttribMax> m_AttribMaxTable;
    std::vector<Parse_CharAttribMax> m_AttribMaxMaxTable;
    std::vector<Parse_CharAttribMax> m_StrengthMaxTable;
    std::vector<Parse_CharAttribMax> m_ResistanceMaxTable;
    template<class Archive>
    void serialize(Archive & archive);
};

using Parse_AllCharClasses = std::vector<CharClass_Data>;