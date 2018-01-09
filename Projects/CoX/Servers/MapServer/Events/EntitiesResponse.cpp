#define _USE_MATH_DEFINES
#include "Events/EntitiesResponse.h"
#include "NetStructures/Powers.h"

#include "Entity.h"
#include "MapEvents.h"
#include "MapClient.h"
#include "MapInstance.h"
#include "EntityUpdateCodec.h"

#include <QByteArray>
#include <cmath>
#include <iso646.h>

namespace  {
struct SurfaceParams
{
    float traction;
    float friction;
    float bounce;
    float gravitational_constant;
    float max_speed;
};
static_assert(2*sizeof(SurfaceParams)==320/8,"Required since it's sent as an bit array");

void storeSuperStats(const EntitiesResponse &/*src*/,BitStream &bs)
{
    bs.StorePackedBits(1,0);
}
void storeGroupDyn(const EntitiesResponse &/*src*/,BitStream &bs)
{
    uint32_t num_graph_nodes_changed=0;
    bs.StorePackedBits(1,num_graph_nodes_changed);
    if(num_graph_nodes_changed==0)
        return;
    if(num_graph_nodes_changed>0)
    {
        for(uint32_t i=0; i<num_graph_nodes_changed; ++i)
        {
            int dynamic_group_idx=0;
            int become_invisible_and_stop_collisions=false;
            bs.StorePackedBits(16,dynamic_group_idx);
            bs.StoreBits(1,become_invisible_and_stop_collisions);
        }
    }
    else
    {
        // if less then zero, then the data will be zlib uncompressed on the client.
        uint8_t packed_group_bits[2] = {0};
        uint8_t dyn_groups=0;
        QByteArray ba = qCompress(packed_group_bits,2);
        ba.remove(0,sizeof(uint32_t)); // qt includes uncompressed size as a first 4 bytes of QByteArray
        bs.StorePackedBits(16,dyn_groups);
        bs.StorePackedBits(10,ba.size());
        bs.StoreBitArray((const uint8_t *)ba.constData(),8*ba.size());
    }
}
void storeTeamList(const EntitiesResponse &/*src*/,BitStream &bs)
{
    int team_id=0; //
    bool mark_lfg=false;
    bool in_mission_or_taskforce_thing=false;
    uint32_t team_leader_id = 0;
    uint32_t team_size=0;
    storePackedBitsConditional(bs,20,team_id);
    bs.StoreBits(1,in_mission_or_taskforce_thing);
    bs.StoreBits(1,mark_lfg);
    if(team_id == 0)
        return;

    bs.StoreBits(32,team_leader_id);
    bs.StorePackedBits(1,team_size);
    for(uint32_t i=0; i<team_size; ++i)
    {
        uint32_t team_member_dbid=0;
        bool team_member_is_on_the_same_map=true;
        bs.StoreBits(32,team_member_dbid);
        bs.StoreBits(1,team_member_is_on_the_same_map);
        if(not team_member_is_on_the_same_map)
        {
            QString missing_team_member_name;
            QString missing_team_member_is_on_map;
            bs.StoreString(missing_team_member_name);
            bs.StoreString(missing_team_member_is_on_map);
        }
    }
}


void serialize_char_full_update(const Entity &src, BitStream &bs )
{
    PUTDEBUG("CharacterFromServer");
    const Character &player_char=src.m_char;
    src.m_char.SendCharBuildInfo(bs); //FIXEDOFFSET_pchar->character_Receive
    PUTDEBUG("PlayerEntity::serialize_full before sendFullStats");
    src.m_char.sendFullStats(bs); //Entity::receiveFullStats(&FullStatsTokens, pak, FIXEDOFFSET_pchar, pkt_id_fullAttrDef, 1);
    PUTDEBUG("PlayerEntity::serialize_full before sendBuffs");
    sendBuffs(src,bs); //FIXEDOFFSET_pchar->character_ReceiveBuffs(pak,0);
    PUTDEBUG("PlayerEntity::serialize_full before sidekick");
    bool has_sidekick=false;
    bs.StoreBits(1,has_sidekick);
    if(has_sidekick)
    {
        bool is_mentor=false; // this flag might mean something totally different :)
        bs.StoreBits(1,is_mentor);
        bs.StorePackedBits(20,0); // sidekick partner db_id -> 10240
    }

    PUTDEBUG("before tray");
    player_char.sendTray(bs);
    PUTDEBUG("before traymode");
    player_char.sendTrayMode(bs);

    bs.StoreString(src.name()); // maxlength 32
    bs.StoreString(src.m_battle_cry); //max 128
    bs.StoreString(src.m_character_description); //max 1024
    PUTDEBUG("before windows");
    player_char.sendWindows(bs);
    bs.StoreBits(1,0); // lfg related
    bs.StoreBits(1,0); // a2->ent_player2->field_AC
    player_char.sendTeamBuffMode(bs);
    player_char.sendDockMode(bs);
    player_char.sendChatSettings(bs);
    player_char.sendTitles(bs);
    player_char.sendDescription(bs);
    uint8_t auth_data[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    PUTDEBUG("before auth data");
    bs.StoreBitArray(auth_data,128);
    player_char.sendKeybinds(bs);
    player_char.sendOptions(bs);
    PUTDEBUG("before friend list");
    player_char.sendFriendList(bs);
}
void storePowerSpec(uint32_t powerset_idx,uint32_t power_idx,BitStream &bs)
{
    bs.StorePackedBits(2,powerset_idx);
    bs.StorePackedBits(1,power_idx);
}
void storePowerInfoUpdate(const EntitiesResponse &/*src*/,BitStream &bs)
{
    bool power_info_updates_available=false;
    bs.StoreBits(1,power_info_updates_available);
    if(power_info_updates_available)
    {
        bool remove_all_powersets_apart_from_inherent=false;
        uint32_t num_powers_to_send=0;
        bs.StoreBits(1,remove_all_powersets_apart_from_inherent);

        bs.StorePackedBits(1,num_powers_to_send);
        for(uint32_t i=0; i<num_powers_to_send; ++i)
        {
            storePowerSpec(0,0,bs);
            bool is_custom_power=false;
            if(is_custom_power)
            {
                uint32_t category_idx=0;
                uint32_t powerset_idx=0;
                uint32_t power_idx=0;
                bs.StorePackedBits(3,category_idx);
                bs.StorePackedBits(3,powerset_idx);
                bs.StorePackedBits(3,power_idx);
                uint32_t maybe_required_level=0;
                uint32_t level_bought=0;
                uint32_t num_charges = 0;
                float usage_time = 0;
                uint32_t timing_rel=0; // timestamp ?
                uint32_t count_boosts=0;
                bs.StorePackedBits(5,maybe_required_level);
                bs.StorePackedBits(5,level_bought);
                bs.StorePackedBits(3,num_charges);
                bs.StoreFloat(usage_time);
                bs.StorePackedBits(24,timing_rel);
                bs.StorePackedBits(4,count_boosts);
                for(uint32_t boost_idx=0; boost_idx<count_boosts; ++boost_idx)
                {
                    bool has_boost = false;
                    bs.StoreBits(1,has_boost);
                    if(has_boost)
                    {
                        uint32_t boost_category_idx=0;
                        uint32_t boost_powerset_idx=0;
                        uint32_t boost_power_idx=0;
                        uint32_t boost_level = 0;
                        uint32_t boost_num_combines=0;
                        bs.StorePackedBits(3,boost_category_idx);
                        bs.StorePackedBits(3,boost_powerset_idx);
                        bs.StorePackedBits(3,boost_power_idx);
                        bs.StorePackedBits(5,boost_level);
                        bs.StorePackedBits(2,boost_num_combines);
                    }
                }
            }
            else
            {
                // nothing to do.
            }
        }
    }
    // sending state of all current powers.
    std::vector<Power> powers;
    for(Power &p : powers)
    {
        bs.StoreBits(1,1); // have power to send.
        uint32_t category_idx=0;
        uint32_t powerset_idx=0;
        uint32_t power_idx=0;
        bs.StorePackedBits(3,category_idx);
        bs.StorePackedBits(3,powerset_idx);
        bs.StorePackedBits(3,power_idx);
        float range=0.0f;
        bs.StoreFloat(range); // nem: I have no idea why it is passed here
    }
    bs.StoreBits(1,0); // no more powers to send.
    if(!power_info_updates_available) // no power list updates were needed
        return;
    uint32_t activation_count=0;
    bs.StorePackedBits(4,activation_count);
    for(uint32_t act=0; act<activation_count; ++act)
    {
        bool active_state_change=false;
        bs.StoreBits(1,active_state_change);
        if(active_state_change)
        {
            uint32_t activation_state=0;
            storePowerSpec(0,0,bs);
            bs.StorePackedBits(1,activation_state);
        }
    }
    uint32_t timer_count=0;
    bs.StorePackedBits(1,timer_count);
    for(uint32_t tmr=0; tmr<timer_count; ++tmr)
    {
        bool timer_updated=false;
        bs.StoreBits(1,timer_updated);
        if(timer_updated)
        {
            float recharge_countdown=0;
            storePowerSpec(0,0,bs);
            bs.StoreFloat(recharge_countdown);
        }
    }
    uint32_t inspiration_count=0;
    storePackedBitsConditional(bs,4,inspiration_count);
    for(uint32_t insp=0; insp<inspiration_count; ++insp)
    {
        //TODO: fill this
        assert(false);
    }
    uint32_t boost_count=0;
    bs.StorePackedBits(1,boost_count);
    for(uint32_t insp=0; insp<inspiration_count; ++insp)
    {
        //TODO: fill this
        assert(false);
    }
}
void sendServerControlState(const EntitiesResponse &src,BitStream &bs)
{
    glm::vec3 spd(1,1,1);
    glm::vec3 zeroes;
    bool m_flying=false;
    bool m_dazed=false;
    // user entity
    Entity *ent = src.m_client->char_entity();
    SurfaceParams surface_params[2];
    memset(&surface_params,0,2*sizeof(SurfaceParams));
    surface_params[0].traction = 1.5f;
    surface_params[0].friction = 1.5f;
    surface_params[0].bounce = 1.5f;
    surface_params[1].max_speed = surface_params[0].max_speed = 1.5f;
    surface_params[1].gravitational_constant = surface_params[0].gravitational_constant = 3.0f;
    uint8_t update_id=1;
    bool update_part_1=true;
    bool update_part_2=false;
    bs.StoreBits(1,update_part_1);
    if(update_part_1)
    {
        //rand()&0xFF
        bs.StoreBits(8,update_id);
        // after input_send_time_initialized, this value is enqueued as CSC_9's control_flags
        // This is entity speed vector !!
        storeVector(bs,spd);

        bs.StoreFloat(1.0f); // speed rel back
        bs.StoreBitArray((uint8_t *)&surface_params,2*sizeof(SurfaceParams)*8);
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
        storeVector(bs,spd);
        storeVectorConditional(bs,zeroes);  // vector3 -> speed ? likely

        storeFloatConditional(bs,0); // Pitch not used ?
        storeFloatConditional(bs,ent->inp_state.camera_pyr.y); // Pitch
        storeFloatConditional(bs,0); // Roll
        bs.StorePackedBits(1,0); // sets the lowest bit in CscCommon::flags
    }
}
void sendServerPhysicsPositions(const EntitiesResponse &src,BitStream &bs)
{
    Entity * target = src.m_client->char_entity();
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
        for(int i=0; i<3; ++i)
            storeFloatConditional(bs,target->vel[i]);
    }
}

void sendControlState(const EntitiesResponse &src,BitStream &bs)
{
    sendServerControlState(src,bs);
}
void sendCommands(const EntitiesResponse &src,BitStream &tgt)
{
    tgt.StorePackedBits(1,1); // use 'time' shortcut
    tgt.StoreFloat(float(src.m_map_time_of_day)*10.0f);
    tgt.StorePackedBits(1,2); // use 'time scale' shortcut
    tgt.StoreFloat(4.0f);
    tgt.StorePackedBits(1,3); // use 'time step scale' shortcut
    tgt.StoreFloat(2.0f);
    tgt.StorePackedBits(1,0);
}
void sendClientData(const EntitiesResponse &src,BitStream &bs)
{
    Entity *ent=src.m_client->char_entity();
    Character &player_char=ent->m_char;
    if(!src.m_incremental)
    {
        //full_update - > receiveCharacterFromServer
        // initial character update = level/name/class/origin/map_name
        //m_client->char_entity()->m_char.m_ent=m_client->char_entity();
        serialize_char_full_update(*ent,bs);
    }
    else
    {
        ent->m_char.sendFullStats(bs);
    }
    storePowerInfoUpdate(src,bs);
    storeTeamList(src,bs);
    storeSuperStats(src,bs);
    storeGroupDyn(src,bs);
    bool additional=false;
    bs.StoreBits(1,additional);
    if(additional)
    {
        bs.StoreFloat(0.1f);
        bs.StoreFloat(0.2f); // camera_yaw
        bs.StoreFloat(0.3f);
    }
}
}
//! EntitiesResponse is sent to a client to inform it about the current world state.
EntitiesResponse::EntitiesResponse(MapClient *cl) :
    MapLinkEvent(MapEventTypes::evEntitites)
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
void EntitiesResponse::serializeto( BitStream &tgt ) const
{
    MapInstance *mi = m_client->current_map();
    EntityManager &ent_manager(mi->m_entities);


    tgt.StorePackedBits(1,m_incremental ? 2 : 3); // opcode  3 - full update.

    tgt.StoreBits(1,ent_major_update); // passed to Entity::EntReceive as a parameter

    sendCommands(*this,tgt);

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
    ent_manager.sendEntities(tgt,m_client,m_incremental);
    if(debug_info&&!unkn2)
    {
        ent_manager.sendDebuggedEntities(tgt); // while loop, sending entity id's and debug info for each
        ent_manager.sendGlobalEntDebugInfo(tgt);
    }
    sendServerPhysicsPositions(*this,tgt); // These are not client specific ?
    sendControlState(*this,tgt);// These are not client specific ?
    ent_manager.sendDeletes(tgt,m_client);
    // Client specific part
    sendClientData(*this,tgt);
    // Server messages follow the entity update.
    auto v= std::move(m_client->m_contents);
    for(const auto &command : v)
        command->serializeto(tgt);
    tgt.StorePackedBits(1,0); // finalize the command list
}

