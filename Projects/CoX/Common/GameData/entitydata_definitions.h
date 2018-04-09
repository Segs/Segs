#pragma once

#include <QtCore/QString>
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>

struct EntityData
{
    static const constexpr  uint32_t    class_version       = 2;
                            uint32_t    m_access_level      = 0;
                            uint8_t     m_origin_idx        = {0};
                            uint8_t     m_class_idx         = {0};
                            glm::vec3   pos;
                            glm::vec3   m_orientation_pyr; // Stored in Radians
};
