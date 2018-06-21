/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <QtCore/QString>
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>

struct EntityData
{
static const constexpr  uint32_t    class_version       = 4;    // v4: adds m_current_map
                        uint32_t    m_access_level      = 0;
                        uint8_t     m_origin_idx        = {0};
                        uint8_t     m_class_idx         = {0};
                        glm::vec3   m_pos;
                        glm::vec3   m_orientation_pyr;          // Stored in Radians
                        uint32_t    m_map_idx           = 1;    // only 1 map instance
                        QString     m_current_map;
};
