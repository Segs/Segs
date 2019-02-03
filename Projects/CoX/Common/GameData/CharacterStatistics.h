/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <QString>
#include "cereal/cereal.hpp"
#include "Logging.h"


//Generic for all statistics. Racing, enemies defeated, inf count, etc.
class CharacterStatistic
{
public:
    enum : uint32_t {class_version       = 1};

    uint32_t m_id; // id for type to update record?
    QString m_name;
    uint64_t m_time;
    uint32_t m_count;

    // for scripting language access.
    std::string getName() const { return m_name.toStdString();}
    void setName(const char *n) { m_name = n; }


    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);
};
using vCharacterStatistics = std::vector<CharacterStatistic>;


