/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "GameData/spawn_definitions.h"
#include <QtCore/QString>
#include <QtCore/QHash>
#include <vector>

struct CritterGenerator
{
    QString                                 m_generator_name;
    QString                                 m_encounter_node_name;
    CritterSpawnLocations                   m_critter_encounter;
    std::vector<CritterSpawnDef>            m_possible_critters_and_groups;
    void generate(class MapInstance *);
};

struct CritterGeneratorStore
{
    QHash<QString, CritterGenerator> m_generators;
    void generate(class MapInstance *instance);
};

QString makeReadableName(QString &name);
