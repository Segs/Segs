#define _USE_MATH_DEFINES
#include "Events/EntitiesResponse.h"

#include "Entity.h"
#include "MapEvents.h"
#include "MapClient.h"
//#include "MapHandler.h"
#include "MapInstance.h"

#include <cmath>

//! EntitiesResponse is sent to a client to inform it about the current world state.
EntitiesResponse::EntitiesResponse(MapClient *cl) :
    MapLinkEvent(MapEventTypes::evEntitites), m_finalized_into(2048)
{
    m_map_time_of_day = 10;
    m_client = cl;
    abs_time=db_time=0;
    unkn2=false;
    debug_info=true;
    m_incremental=false;
    m_interpolating=true;
    m_interpolation_level = 2;
    m_interpolation_bits=1;
    //m_interpolation_level

}
void EntitiesResponse::serializeto_internal( BitStream &tgt ) const
{
    MapInstance *mi = m_client->current_map();
    EntityManager &ent_manager(mi->m_entities);


    tgt.StorePackedBits(1,m_incremental ? 2 : 3); // opcode  3 - full update.

    tgt.StoreBits(1,entReceiveUpdate); // passed to Entity::EntReceive as a parameter

    sendCommands(tgt);

    tgt.StoreBits(32,abs_time);
    //tgt.StoreBits(32,db_time);

    tgt.StoreBits(1,unkn2);
    if(unkn2)
    {
        //g_debug_info 0
        //interpolation level 2
        //g_bitcount_rel 1
    }
    else
    {
        tgt.StoreBits(1,debug_info);
        tgt.StoreBits(1,m_interpolating);
        if(m_interpolating==1)
        {
            tgt.StoreBits(2,m_interpolation_level);
            tgt.StoreBits(2,m_interpolation_bits);
        }
    }
    ;
    //else debug_info = false;
    ent_manager.sendEntities(tgt,m_client->char_entity()->getIdx(),m_incremental);
    if(debug_info&&!unkn2)
    {
        ent_manager.sendDebuggedEntities(tgt); // while loop, sending entity id's and debug info for each
        ent_manager.sendGlobalEntDebugInfo(tgt);
    }
    sendServerPhysicsPositions(tgt); // These are not client specific ?
    sendControlState(tgt);// These are not client specific ?
    ent_manager.sendDeletes(tgt);
    // Client specific part
    sendClientData(tgt);
    //FIXME: Most Server messages must follow entity update.
}
void EntitiesResponse::sendClientData(BitStream &tgt) const
{
    PlayerEntity *ent=static_cast<PlayerEntity *>(m_client->char_entity());
    Character &player_char=ent->m_char;
    if(!m_incremental)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\tSending Character to client: full\n")));

        //full_update - > receiveCharacterFromServer
        // initial character update = level/name/class/origin/map_name
        //m_client->char_entity()->m_char.m_ent=m_client->char_entity();
        ent->serialize_full(tgt);
        player_char.sendTray(tgt);
        player_char.sendTrayMode(tgt);
        tgt.StoreString(ent->name()); // maxlength 32

        tgt.StoreString(ent->m_battle_cry); //max 128
        tgt.StoreString(ent->m_character_description); //max 1024
        player_char.sendWindows(tgt);
        tgt.StoreBits(1,0); // lfg related
        tgt.StoreBits(1,0); // a2->ent_player2->field_AC
        player_char.sendTeamBuffMode(tgt);
        player_char.sendDockMode(tgt);
        player_char.sendChatSettings(tgt);
        player_char.sendTitles(tgt);
        player_char.sendDescription(tgt);
        uint8_t auth_data[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        tgt.StoreBitArray(auth_data,128);
        player_char.sendKeybinds(tgt);
        player_char.sendOptions(tgt);
        player_char.sendFriendList(tgt);
    }
    else
    {
        //ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\tSending Character to client: stats-only\n")));
        ent->m_char.sendFullStats(tgt);
    }
    storePowerInfoUpdate(tgt);
    //storePowerModeUpdate(tgt);
    //storeBadgeUpdate(tgt);
    //storeGenericinventoryUpdate(tgt);
    //storeInventionUpdate(tgt);
    storeTeamList(tgt);
    storeSuperStats(tgt);
    storeGroupDyn(tgt);
    bool additional=false;
    tgt.StoreBits(1,additional);
    if(additional)
    {
        tgt.StoreFloat(0.1f);
        tgt.StoreFloat(0.2f); // camera_yaw
        tgt.StoreFloat(0.3f);
    }
}
void EntitiesResponse::sendControlState(BitStream &bs) const
{
    sendServerControlState(bs);
}
void EntitiesResponse::sendServerPhysicsPositions(BitStream &bs) const
{
    Entity * target = m_client->char_entity();
    bool full_update = true;
    bool has_control_id = true;

    bs.StoreBits(1,full_update);
    if( !full_update )
        bs.StoreBits(1,has_control_id);
#ifdef LOG_
    fprintf(stderr,"Phys: send %d ",target->m_input_ack);
#endif
    if( full_update || has_control_id)
        bs.StoreBits(16,target->m_input_ack); //target->m_input_ack
    if(full_update)
    {
        for(int i=0; i<3; ++i)
            bs.StoreFloat(target->pos[i]); // server position
        NetStructure::storeFloatConditional(bs,0.0f); // PYR rotation ?
        NetStructure::storeFloatConditional(bs,0.0f);
        NetStructure::storeFloatConditional(bs,0.0f);
    }
}
struct EntWalkRel
{
    float v[3];
    float gravitational_constant;
    float max_speed;
};
struct CscCommon_Sub28
{
    EntWalkRel a,b;
};
//static_assert(sizeof(CscCommon_Sub28)==320/8,"Required since it's sent as an bit array");
void EntitiesResponse::sendServerControlState(BitStream &bs) const
{
    glm::vec3 spd(1,1,1);
    glm::vec3 zeroes;
    bool m_flying=false;
    bool m_dazed=false;
    // user entity
    Entity *ent = m_client->char_entity();
    CscCommon_Sub28 struct_csc;
    memset(&struct_csc,0,sizeof(struct_csc));
    for(int i=0; i<3; ++i)
        struct_csc.a.v[i] = 1.5;
    struct_csc.b.max_speed = struct_csc.a.max_speed = 1.5f;
    struct_csc.b.gravitational_constant = struct_csc.a.gravitational_constant = 3.0f;
    //    for(int i=3; i<5; ++i)
    //        struct_csc.a.v[i] = rand()&0xf;
    bool update_part_1=true;
    bool update_part_2=false;
    bs.StoreBits(1,update_part_1);
    if(update_part_1)
    {
        //rand()&0xFF
        bs.StoreBits(8,1); // value stored in control state field_134
        // after input_send_time_initialized, this value is enqueued as CSC_9's control_flags
        // This is entity speed vector !!
        NetStructure::storeVector(bs,spd);

        bs.StoreFloat(1.0f); // speed rel back
        bs.StoreBitArray((uint8_t *)&struct_csc,sizeof(CscCommon_Sub28)*8);
        bs.StoreFloat(0.1f);
        bs.StoreBits(1,m_flying); // key push bits ??
        bs.StoreBits(1,m_dazed); // key push bits ??
        bs.StoreBits(1,0); // key push bits ??
        bs.StoreBits(1,0); // key push bits ??
        bs.StoreBits(1,0); // key push bits ??
        bs.StoreBits(1,0); // key push bits ??
    }
    // Used to force the client to a position/speed/pitch/rotation by server
    bs.StoreBits(1,update_part_2);
    if(update_part_2)
    {
        bs.StorePackedBits(1,0); // sets g_client_pos_id_rel
        NetStructure::storeVector(bs,spd);
        NetStructure::storeVectorConditional(bs,zeroes);  // vector3 -> speed ? likely

        NetStructure::storeFloatConditional(bs,0); // Pitch not used ?
        NetStructure::storeFloatConditional(bs,ent->inp_state.camera_pyr.y); // Pitch
        NetStructure::storeFloatConditional(bs,0); // Roll
        bs.StorePackedBits(1,0); // sets the lowest bit in CscCommon::flags
    }
}
void EntitiesResponse::storePowerInfoUpdate(BitStream &bs) const
{
    bool powerinfo_available=false;
    bs.StoreBits(1,powerinfo_available);
    if ( powerinfo_available )
    {
        bs.StoreBits(1,false);
        bs.StorePackedBits(1,0);
        // send powers
        bs.StoreBits(1,0);

        bs.StorePackedBits(4,0);
        bs.StorePackedBits(1,0);
        // ... missing
    }
    // send powers
    bs.StoreBits(1,0);
}
void EntitiesResponse::storePowerModeUpdate(BitStream &bs) const
{
    bs.StoreBits(1,0);
    if(false)
    {
        bs.StorePackedBits(3,1);
        for(int i=0; i<1; i++)
        {
            bs.StorePackedBits(3,0);
        }
    }
}
void EntitiesResponse::storeBadgeUpdate(BitStream &bs) const
{
    bs.StoreBits(1,0);
    bs.StoreBits(1,0);
}
void EntitiesResponse::storeGenericinventoryUpdate(BitStream &bs)const
{
    bs.StorePackedBits(1,0);
}
void EntitiesResponse::storeInventionUpdate(BitStream &bs)const
{
    bs.StorePackedBits(1,0);
}
void EntitiesResponse::storeTeamList(BitStream &bs) const
{
    int team_id=0; //
    NetStructure::storePackedBitsConditional(bs,20,team_id);
    //storePackedBitsConditional(bs,20,0);
    bs.StoreBits(1,0);
    bs.StoreBits(1,0);
    if(team_id>0)
    {
        bs.StoreBits(32,0);
        int num_members=0;
        bs.StorePackedBits(1,num_members);
    }
}
void EntitiesResponse::storeSuperStats(BitStream &bs) const
{
    bs.StorePackedBits(1,0);
}
void EntitiesResponse::storeGroupDyn(BitStream &bs) const
{
    bs.StorePackedBits(1,0);
    // if less then zero, then the data will be zlib uncompressed on the client.
}

