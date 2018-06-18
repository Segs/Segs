/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <glm/vec2.hpp>
#include <QtCore/QString>
#include <vector>

struct Map_Data
{
    glm::vec2 Location;
    glm::vec2 TextLocation;
    QString Name;
    QString Icon;
};
typedef std::vector<Map_Data> AllMaps_Data;
