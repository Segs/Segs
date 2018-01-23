#pragma once

#include <QtCore/QString>

struct EntityData
{
    uint8_t             m_origin_idx                = {0};
    uint8_t             m_class_idx                 = {0};
    uint8_t             m_type                      = {0};
    int32_t             m_idx                       = {0};
    uint32_t            m_db_id                     = {0};
};
