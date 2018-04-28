/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once

#include "glm/mat4x4.hpp"

#include <QtCore/QString>
#include <QtCore/QHash>

#include <vector>

struct NpcGenerator
{
    QString costume_name;
    std::vector<glm::mat4> initial_positions;
    void generate(class MapInstance *);
};

struct NpcGeneratorStore
{
    QHash<QString,NpcGenerator> m_generators;
    void generate(class MapInstance *instance);
};
