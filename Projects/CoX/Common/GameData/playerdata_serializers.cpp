/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "playerdata_serializers.h"
#include "playerdata_definitions.h"
#include "serialization_common.h"

#include "keybind_serializers.h"
#include "gui_serializers.h"
#include "clientoptions_serializers.h"

#include "Logging.h"

CEREAL_CLASS_VERSION(PlayerData, PlayerData::class_version)  // register PlayerData class version

const constexpr uint32_t PlayerData::class_version;

template<class Archive>
void serialize(Archive &archive, PlayerData &cd, uint32_t const version)
{
    if (version != cd.class_version)
    {
        qCritical() << "Failed to serialize PlayerData, incompatible serialization format version " << version;
        return;
    }
    archive(cereal::make_nvp("Gui",cd.m_gui));
    archive(cereal::make_nvp("KeyBinds",cd.m_keybinds));
    archive(cereal::make_nvp("Options",cd.m_options));
}

SPECIALIZE_VERSIONED_SERIALIZATIONS(PlayerData);

//! @}
