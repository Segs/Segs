/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "CommonNetStructures.h"
#include "glm/vec3.hpp"
#include "cereal/cereal.hpp"
#include "Logging.h"

class Location
{
public:
    enum : uint32_t {class_version       = 1};

    QString         m_location_name;
    glm::vec3       m_location_coordinates;

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);

};

using vLocationList = std::vector<Location>;
