/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "EntitiesResponse.h"

#include "GameData/Powers.h"
#include "GameData/Entity.h"
#include "GameData/Character.h"
#include "GameData/CharacterHelpers.h"
#include "GameData/Movement.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/map_definitions.h"
#include "MapEvents.h"
#include "Components/Logging.h"

#include <QByteArray>
#include <glm/ext.hpp>
#include <cmath>
#ifdef _MSC_VER
#include <iso646.h>
#endif

using namespace SEGSEvents;


//! EntitiesResponse is sent to a client to inform it about the current world state.
EntitiesResponse::EntitiesResponse() : MapLinkEvent(evEntitiesResponse)
{
    m_map_time_of_day       = 10;
    g_interpolation_level   = 2;
    g_interpolation_bits    = 1;
}

void EntitiesResponse::serializefrom(BitStream &src)
{
    ent_major_update = src.GetBits(1);
    assert(false);
}

void EntitiesResponse::serializeto(BitStream &tgt) const
{
    tgt.appendBitStream(blob_of_death);
}

//! @}
