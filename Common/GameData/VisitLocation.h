/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "CommonNetStructures.h"
#include "glm/vec3.hpp"
#include "cereal/cereal.hpp"
#include "Components/Logging.h"

class VisitLocation
{
public:
    enum : uint32_t {class_version       = 1};

    QString         m_location_name;
    glm::vec3       m_pos;

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);
    VisitLocation(){}
    VisitLocation(QString name, glm::vec3 pos)
    {
        m_location_name = name;
        m_pos = pos;
    }


};

using vLocationList = std::vector<VisitLocation >;
