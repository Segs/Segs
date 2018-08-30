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

const constexpr uint32_t PlayerData::class_version;
CEREAL_CLASS_VERSION(PlayerData, PlayerData::class_version)         // register PlayerData class version

template<class Archive>
void serialize(Archive &archive, PlayerData &pd, uint32_t const version)
{
    if (version != pd.class_version)
    {
        qCritical() << "Failed to serialize PlayerData, incompatible serialization format version " << version;
        return;
    }
    archive(cereal::make_nvp("AuthData", pd.m_auth_data));
    archive(cereal::make_nvp("Gui", pd.m_gui));
    archive(cereal::make_nvp("KeyBinds", pd.m_keybinds));
    archive(cereal::make_nvp("Options", pd.m_options));
}

template
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive, PlayerData & m, uint32_t const version);
template
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive, PlayerData & m, uint32_t const version);

//! @}
