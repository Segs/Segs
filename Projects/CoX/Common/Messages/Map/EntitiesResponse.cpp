/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
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
//#include "MapClientSession.h"
#include "MapEvents.h"
//#include "MapInstance.h"
//#include "EntityUpdateCodec.h"
//#include "DataHelpers.h"
#include "Logging.h"

#include <QByteArray>
#include <glm/ext.hpp>
#include <cmath>
#ifdef _MSC_VER
#include <iso646.h>
#endif

using namespace SEGSEvents;


//! EntitiesResponse is sent to a client to inform it about the current world state.
EntitiesResponse::EntitiesResponse() :
    MapLinkEvent(MapEventTypes::evEntitiesResponse)
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
void EntitiesResponse::serializeto( BitStream &tgt ) const
{
    tgt.StorePackedBits(1,m_incremental ? 2 : 3); // opcode  3 - full update.

    tgt.StoreBits(1,ent_major_update); // passed to Entity::EntReceive as a parameter

    tgt.appendBitStream(commands);

    tgt.StoreBits(32,abs_time);
    //tgt.StoreBits(32,db_time);
    bool all_defaults = (debug_info==0) && (g_interpolation_level==2) && (g_interpolation_bits==1);
    tgt.StoreBits(1,all_defaults);
    if(!all_defaults)
    {
        tgt.StoreBits(1,debug_info);
        tgt.StoreBits(1,g_interpolation_level!=0);
        if(g_interpolation_level!=0)
        {
            tgt.StoreBits(2,g_interpolation_level);
            tgt.StoreBits(2,g_interpolation_bits);
        }
    }
    ;
    
    tgt.appendBitStream(entities_update);
    tgt.appendBitStream(physics_update);
    tgt.appendBitStream(controls_update);
    tgt.appendBitStream(entity_removals);
    // Client specific part
    tgt.appendBitStream(client_data);
    tgt.appendBitStream(follow_up_commands);
}

//! @}
