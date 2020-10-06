/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <QString>
#include "cereal/cereal.hpp"
#include "Components/Logging.h"

struct HideAndSeek
{
    enum : uint32_t {class_version       = 1};

    uint16_t m_found_count;

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);
};

struct RelayRaceResult
{
    enum : uint32_t {class_version       = 1};

    uint16_t m_segment;
    uint32_t m_last_time;
    uint32_t m_best_time;

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);
};

class Hunt
{
public:
    enum : uint32_t {class_version       = 1};

    QString m_type; // Skulls, hellions, etc?
    uint32_t m_count;

    //For scripting access
    std::string getTypeString() const { return m_type.toStdString();}
    void setTypeString(const char *n) { m_type = n; }

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);
};

using vRelayRace = std::vector<RelayRaceResult>;
using vHunt = std::vector<Hunt>;

//Generic for all statistics. Racing, enemies defeated, inf count, etc.
struct PlayerStatistics
{
    enum : uint32_t {class_version       = 2};

    HideAndSeek     m_hide_seek;
    vRelayRace      m_relay_races;
    vHunt           m_hunts;

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);
};
