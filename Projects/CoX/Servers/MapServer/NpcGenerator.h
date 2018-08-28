/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "glm/mat4x4.hpp"

#include <QtCore/QString>
#include <QtCore/QHash>

#include <vector>

enum class EntType : uint8_t;
struct NpcGenerator
{
    QString costume_name;
    EntType type;
    std::vector<glm::mat4> initial_positions;
    void generate(class MapInstance *);
};

struct NpcGeneratorStore
{
    QHash<QString,NpcGenerator> m_generators;
    void generate(class MapInstance *instance);
};

QString makeReadableName(QString &name);
