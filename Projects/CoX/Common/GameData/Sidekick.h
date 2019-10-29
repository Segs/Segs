/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <stdint.h>
enum SidekickType : uint8_t
{
    NoSidekick  = 0,
    IsSidekick  = 1,
    IsMentor    = 2,
};

struct Sidekick
{
    enum : uint32_t { class_version = 1 };
    
    bool                        m_has_sidekick = false;
    uint32_t                    m_db_id        = 0;
    SidekickType                m_type         = NoSidekick;
    template <class Archive> 
    void                        serialize(Archive &archive, uint32_t const version);
};
