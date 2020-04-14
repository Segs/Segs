/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <glm/vec3.hpp>
#include "glm/common.hpp"


namespace SEGS
{
struct AxisAlignedBoundingBox
{
    void merge(glm::vec3 pt)
    {
        m_min = glm::min(m_min,pt);
        m_max = glm::max(m_max,pt);
    }
    void merge(AxisAlignedBoundingBox box)
    {
        m_min = glm::min(m_min,box.m_min);
        m_max = glm::max(m_max,box.m_max);
    }
    void clear()
    {
        m_min = m_max = {0,0,0};
    }
    glm::vec3 size() const { return m_max - m_min; }
    glm::vec3 center()const { return (m_max+m_min)/0.5f;}
    glm::vec3 m_min;
    glm::vec3 m_max;
};

} // namespace SEGS
