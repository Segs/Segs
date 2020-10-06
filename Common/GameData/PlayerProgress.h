/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <QString>
#include "cereal/cereal.hpp"
#include "Components/Logging.h"

struct PlayerProgress
{
    enum : uint32_t {class_version = 1};
    std::map<uint32_t, std::vector<bool> > m_visible_map_cells;

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);
};
