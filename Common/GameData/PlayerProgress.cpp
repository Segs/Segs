/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "PlayerProgress.h"
#include "Components/Logging.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include "cereal/types/map.hpp"
#include "cereal/types/vector.hpp"

template<class Archive>
void PlayerProgress::serialize(Archive &archive, uint32_t const version)
{
    if(version != PlayerProgress::class_version)
    {
        qCritical() << "Failed to serialize PlayerProgress, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("VisibleMapCells", m_visible_map_cells));
}

CEREAL_CLASS_VERSION(PlayerProgress, PlayerProgress::class_version);
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(PlayerProgress);
