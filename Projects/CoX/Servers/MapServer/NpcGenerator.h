/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "glm/mat4x4.hpp"

#include <QtCore/QString>
#include <QtCore/QHash>

#include <vector>

enum class EntType : uint8_t;
class World;
struct NpcTemplate
{
    QString m_costume_name;
    EntType m_type;
    std::vector<glm::mat4> m_initial_positions;
    // attributes/powers ?
};

struct NpcGenerator
{
    QString m_generator_name;
    EntType m_type;
    std::vector<glm::mat4> m_initial_positions;
    std::vector<NpcTemplate> m_possible_npcs;
    void generate(World *instance);
};

struct NpcGeneratorStore
{
    QHash<QString, NpcGenerator> m_generators;
    void generate(World *instance);
};

QString makeReadableName(QString &name);
