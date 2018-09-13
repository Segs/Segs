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

#include "NetStructures/Powers.h"
#include "NetStructures/Entity.h"
#include "NetStructures/Character.h"
#include "NetStructures/CharacterHelpers.h"
#include "NetStructures/Movement.h"
#include "NetStructures/StateStorage.h"
#include "GameData/playerdata_definitions.h"
#include "MapClientSession.h"
#include "MapEvents.h"
#include "MapInstance.h"
#include "EntityUpdateCodec.h"
#include "DataHelpers.h"
#include "Logging.h"

#include <QByteArray>
#include <glm/ext.hpp>
#include <cmath>
#ifdef _MSC_VER
#include <iso646.h>
#endif

using namespace SEGSEvents;

namespace  {

static_assert(2*sizeof(SurfaceParams)==320/8,"Required since it's sent as an bit array");

void storeSuperStats(const EntitiesResponse &/*src*/,BitStream &bs)
{
    bs.StorePackedBits(1,0);
}

void storeGroupDyn(const EntitiesResponse &/*src*/,BitStream &bs)
{
    // FixMe: num_graph_nodes_changed is initialized and never modified afterward, so the if/else is never reached.
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

void storeTeamList(const EntitiesResponse &src,BitStream &bs)
{
    Entity *e = src.m_client->m_ent;
    assert(e);

    // shorthand local vars
    int         team_idx = 0;
    bool        mark_lfg = e->m_char->m_char_data.m_lfg;
    bool        has_mission = false;
    uint32_t    tm_leader_id = 0;
    uint32_t    tm_size = 0;

    if(e->m_has_team && e->m_team != nullptr)
    {
        team_idx        = e->m_team->m_team_idx;
        has_mission     = e->m_team->m_team_has_mission;
        tm_leader_id    = e->m_team->m_team_leader_idx;
        tm_size         = e->m_team->m_team_members.size();
    }

    storePackedBitsConditional(bs,20,team_idx);
    bs.StoreBits(1,has_mission);
    bs.StoreBits(1,mark_lfg);

    if(team_idx == 0) // if no team, return.
        return;

    bs.StoreBits(32,tm_leader_id); // must be db_id
    bs.StorePackedBits(1,tm_size);

    for(const auto &member : e->m_team->m_team_members)
    {
        Entity *tm_ent = getEntityByDBID(src.m_client->m_current_map, member.tm_idx);

        if(tm_ent == nullptr)
        {
            qWarning() << "Could not find Entity with db_id:" << member.tm_idx;
            continue; // continue loop
        }

        QString member_name     = tm_ent->name();
        QString member_mapname  = getEntityDisplayMapName(tm_ent->m_entity_data);
        bool tm_on_same_map     = tm_ent->m_entity_data.m_map_idx == e->m_entity_data.m_map_idx;

        bs.StoreBits(32, member.tm_idx);
        bs.StoreBits(1, tm_on_same_map);

        if(!tm_on_same_map)
        {
            bs.StoreString(member_name);
            bs.StoreString(member_mapname);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start of PlayerData network serialization
//                GUI
void sendChatSettings(const GUISettings &gui,BitStream &bs)
{
    //int i;
    bs.StoreFloat(gui.m_chat_divider_pos); // chat divider position
    bs.StorePackedBits(1,gui.m_chat_top_flags); // bitmask of channels (top window )
    bs.StorePackedBits(1,gui.m_chat_bottom_flags); // bitmask of channels (bottom )
    bs.StorePackedBits(1,gui.m_cur_chat_channel); // selected channel, Local=10, 11 broadcast,
}

static void sendWindow(BitStream &bs,const GUIWindow &wnd)
{
    qCDebug(logGUI) << "sendWindow:" << wnd.m_idx;
    if(logGUI().isDebugEnabled())
        wnd.guiWindowDump();

    bs.StorePackedBits(1,wnd.m_posx);
    bs.StorePackedBits(1,wnd.m_posy);
    bs.StorePackedBits(1,wnd.m_mode);
    bs.StorePackedBits(1,wnd.m_locked);
    bs.StorePackedBits(1,wnd.m_color);
    bs.StorePackedBits(1,wnd.m_alpha);
    bs.StoreBits(1,wnd.m_draggable_frame);
    if(wnd.m_draggable_frame)
    {
        bs.StorePackedBits(1,wnd.m_width);
        bs.StorePackedBits(1,wnd.m_height);
    }
}

static void sendWindows(const GUISettings &gui, BitStream &bs )
{
    for(uint32_t i=0; i<35; i++)
    {
        bs.StorePackedBits(1,i); // window index
        sendWindow(bs, gui.m_wnds.at(i));
    }
}

static void sendTeamBuffMode(const GUISettings &gui,BitStream &bs)
{
    bs.StoreBits(1,gui.m_team_buffs);
}

static void sendDockMode(const GUISettings &gui, BitStream &bs)
{
    bs.StoreBits(32,gui.m_tray1_number); // Tray #1 Page
    bs.StoreBits(32,gui.m_tray2_number); // Tray #2 Page
}

static void sendTrayMode(const GUISettings &gui, BitStream &bs)
{
    bs.StoreBits(1,gui.m_powers_tray_mode);
}

//////////////////////////////
//                Keybinds
static void sendKeybinds(const KeybindSettings &keybinds,BitStream &bs)
{
    const CurrentKeybinds &cur_keybinds = keybinds.getCurrentKeybinds();
    int total_keybinds = cur_keybinds.size();

    qCDebug(logKeybinds) << "total keybinds:" << total_keybinds;

    bs.StoreString(keybinds.m_cur_keybind_profile); // keybinding profile name

    for(int i=0; i<COH_INPUT_LAST_NON_GENERIC; ++i)
    {
      if(i < total_keybinds) // i begins at 0
      {
         const Keybind &kb(cur_keybinds.at(i));
         bs.StoreString(kb.Command);

         if(kb.IsSecondary)
         {
             int32_t sec = (kb.Key | 0xF00);
             bs.StoreBits(32,sec);
             qCDebug(logKeybinds) << "is secondary:" << sec;
         }
         else
             bs.StoreBits(32,kb.Key);

         bs.StoreBits(32,kb.Mods);
         qCDebug(logKeybinds) << i << kb.KeyString << kb.Key << kb.Mods << kb.Command << " secondary:" << kb.IsSecondary;
      }
      else
      {
         bs.StoreString("");
         bs.StoreBits(32,0);
         bs.StoreBits(32,0);
         qCDebug(logKeybinds) << i;
      }
    }
}

static void sendOptionsFull(const ClientOptions &options,BitStream &bs)
{
    bs.StoreFloat(options.m_mouse_speed);
    bs.StoreFloat(options.m_turn_speed);
    bs.StoreBits(1,options.m_mouse_invert);
    bs.StoreBits(1,options.m_fade_chat_wnd);
    bs.StoreBits(1,options.m_fade_nav_wnd);
    bs.StoreBits(1,options.m_show_tooltips);
    bs.StoreBits(1,options.m_allow_profanity);
    bs.StoreBits(1,options.m_chat_balloons);

    bs.StoreBits(3,options.m_show_archetype);
    bs.StoreBits(3,options.m_show_supergroup);
    bs.StoreBits(3,options.m_show_player_name);
    bs.StoreBits(3,options.m_show_player_bars);
    bs.StoreBits(3,options.m_show_enemy_name);
    bs.StoreBits(3,options.m_show_enemy_bars);
    bs.StoreBits(3,options.m_show_player_reticles);
    bs.StoreBits(3,options.m_show_enemy_reticles);
    bs.StoreBits(3,options.m_show_assist_reticles);

    bs.StorePackedBits(5,options.m_chat_font_size); // value only used on client if >=5
}

static void sendOptions(const ClientOptions &options, bool send_full, BitStream &bs )
{
    bs.StoreBits(1,send_full);
    if(send_full)
    {
        sendOptionsFull(options,bs);
    }
    else
    {
        bs.StoreBits(1,options.m_mouse_invert);
        bs.StoreFloat(options.m_mouse_speed);
        bs.StoreFloat(options.m_turn_speed);
    }
    bs.StoreBits(1,options.m_first_person_view);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void serialize_char_full_update(const Entity &src, BitStream &bs )
{
    PUTDEBUG("CharacterFromServer");
    const Character &player_char(*src.m_char);
    const PlayerData &player_data(*src.m_player);
    src.m_char->SendCharBuildInfo(bs); //FIXEDOFFSET_pchar->character_Receive
    PUTDEBUG("PlayerEntity::serialize_full before sendFullStats");
    src.m_char->sendFullStats(bs); //Entity::receiveFullStats(&FullStatsTokens, pak, FIXEDOFFSET_pchar, pkt_id_fullAttrDef, 1);
    PUTDEBUG("PlayerEntity::serialize_full before sendBuffs");
    sendBuffs(src,bs); //FIXEDOFFSET_pchar->character_ReceiveBuffs(pak,0);

    PUTDEBUG("PlayerEntity::serialize_full before sidekick");
    bs.StoreBits(1,player_char.m_char_data.m_sidekick.m_has_sidekick);
    if(player_char.m_char_data.m_sidekick.m_has_sidekick)
    {
        bool is_mentor = isSidekickMentor(src);
        bs.StoreBits(1,is_mentor);
        bs.StorePackedBits(20,player_char.m_char_data.m_sidekick.m_db_id); // sidekick partner db_id -> 10240
    }

    PUTDEBUG("before tray");
    player_char.m_char_data.m_trays.serializeto(bs);
    PUTDEBUG("before traymode");
    sendTrayMode(player_data.m_gui, bs);

    bs.StoreString(src.name());                     // maxlength 32
    bs.StoreString(getBattleCry(player_char));      // max 128
    bs.StoreString(getDescription(player_char));    // max 1024
    PUTDEBUG("before windows");
    sendWindows(player_data.m_gui, bs);
    bs.StoreBits(1,player_char.m_char_data.m_lfg);              // lfg related
    bs.StoreBits(1,player_char.m_char_data.m_using_sg_costume); // SG mode
    sendTeamBuffMode(player_data.m_gui,bs);
    sendDockMode(player_data.m_gui,bs);
    sendChatSettings(player_data.m_gui,bs);
    player_char.sendTitles(bs,NameFlag::NoName,ConditionalFlag::Unconditional); // NoName, we already sent it above.

    if(src.m_has_owner)
    {
        // TODO: if has owner, send again using owner info (desc first this time)
        bs.StoreString(getDescription(player_char));    // max 1024
        bs.StoreString(getBattleCry(player_char));      // max 128
    }
    else
    {
        // client expects this anyway, so let's send again (desc first this time)
        bs.StoreString(getDescription(player_char));    // max 1024
        bs.StoreString(getBattleCry(player_char));      // max 128
    }

    PUTDEBUG("before auth data");
    bs.StoreBitArray(player_data.m_auth_data,128);
    sendKeybinds(player_data.m_keybinds,bs);
    sendOptions(player_data.m_options,false,bs);
    PUTDEBUG("before friend list");
    player_char.sendFriendList(bs);
}

void storePowerSpec(uint32_t powerset_idx,uint32_t power_idx,BitStream &bs)
{
    bs.StorePackedBits(2,powerset_idx);
    bs.StorePackedBits(1,power_idx);
}

void storePowerRanges(const CharacterData &cd, BitStream &bs)
{
    // sending state of all current powers.
    for(const CharacterPowerSet &pset : cd.m_powersets)
    {
        for(const CharacterPower &p : pset.m_powers)
        {
            //qCDebug(logPowers) << "Sending Power:" << p.m_name << pset.m_index << p.m_index;

            bs.StoreBits(1,1); // have power to send.
            p.m_power_info.serializeto(bs);
            bs.StoreFloat(p.m_range); // nem: I have no idea why it is passed here
        }
    }
    bs.StoreBits(1,0); // no more powers to send.
    //qCDebug(logPowers) << "No more powers to send.";
}

void storePowerInfoUpdate(const GameDataStore &data, const EntitiesResponse &src, BitStream &bs)
{
    Entity *e = src.m_client->m_ent;
    assert(e);
    CharacterData *cd = &e->m_char->m_char_data;

    bs.StoreBits(1, cd->m_powers_updated);
    if(!cd->m_powers_updated)
    {
        storePowerRanges(*cd, bs);
        return;
    }
    qCDebug(logPowers) << "Powers Updated:" << cd->m_powers_updated;


    // Reset Powersets (for respec)
    bs.StoreBits(1, cd->m_reset_powersets);
    if(cd->m_reset_powersets)
    {
        qCDebug(logPowers) << "Resetting Powers:" << cd->m_reset_powersets;
        CharacterPowerSet temp;
        uint32_t pcat_idx = getPowerCatByName(data,"Temporary_Powers");

        for(CharacterPowerSet &pset : cd->m_powersets)
        {
            if(pset.m_category == pcat_idx)
            {
                temp = pset;
                break;
            }
        }

        if(temp.m_powers.empty())
            qWarning() << "Failed to find Temporary_Powers PowerSet";
        else
        {
            cd->m_powersets.clear();
            cd->m_powersets.push_back(temp);
        }

        cd->m_reset_powersets = false;
    }

    // Count all powers owned
    uint32_t total_powers_owned = 0;
    for(const CharacterPowerSet &pset : cd->m_powersets)
    {
        total_powers_owned += pset.m_powers.size(); // total up all powers
    }

    qCDebug(logPowers) << "Update Powers:" << total_powers_owned;
    bs.StorePackedBits(1, total_powers_owned);
    int i = 0;
    for(const CharacterPowerSet &pset : cd->m_powersets)
    {
        int j = 0;
        for(const CharacterPower &power : pset.m_powers)
        {
            qCDebug(logPowers) << "Power:" << power.m_name << pset.m_index << power.m_index;
            storePowerSpec(i, j, bs);

            bs.StoreBits(1, !power.m_erase_power);
            if(power.m_erase_power)
            {
                qCDebug(logPowers) << "  Removing power" << power.m_name << pset.m_index << power.m_index;
                removePower(*cd, power.m_power_info);
                continue;
            }

            power.m_power_info.serializeto(bs);
            bs.StorePackedBits(5, pset.m_level_bought);
            bs.StorePackedBits(5, power.m_level_bought);
            bs.StorePackedBits(3, power.m_num_charges);
            bs.StoreFloat(power.m_usage_time);
            bs.StorePackedBits(24, power.m_activation_time);

            qCDebug(logPowers) << "  NumOfEnhancements:" << power.m_total_eh_slots;
            bs.StorePackedBits(4, power.m_total_eh_slots); // total owned enhancement slots
            for(uint32_t i = 0; i < power.m_total_eh_slots; ++i)
            {
                if(power.m_enhancements.empty() || power.m_enhancements[i].m_name.isEmpty())
                {
                    qCDebug(logPowers) << "  No Enhancement:" << i;
                    bs.StoreBits(1, false);
                    continue;
                }

                qCDebug(logPowers) << "  Enhancement:" << power.m_enhancements[i].m_name
                                   << power.m_enhancements[i].m_slot_idx
                                   << power.m_enhancements[i].m_slot_used;
                bs.StoreBits(1, power.m_enhancements[i].m_slot_used);        // slot has enhancement
                if(power.m_enhancements[i].m_slot_used)
                {
                    power.m_enhancements[i].m_enhance_info.serializeto(bs);
                    bs.StorePackedBits(5, power.m_enhancements[i].m_level);
                    bs.StorePackedBits(2, power.m_enhancements[i].m_num_combines);
                }
            }
            ++j;
        }
        ++i;
    }

    qCDebug(logPowers) << "Send State of All Powers";
    storePowerRanges(*cd, bs); // sending state of all current powers.

    qCDebug(logPowers) << "NumQueuedPowers:" << e->m_queued_powers.size();
    bs.StorePackedBits(4, e->m_queued_powers.size()); // Count all active powers
    for(const CharacterPower *pow : e->m_queued_powers)
    {
        qCDebug(logPowers) << "  QueuedPower:" << pow->m_name << pow->m_index;
        bs.StoreBits(1, pow->m_active_state_change);
        if(pow->m_active_state_change)
        {
            storePowerSpec(pow->m_power_info.m_pset_idx, pow->m_index, bs);
            bs.StorePackedBits(1, pow->m_activation_state);
        }
    }

    qCDebug(logPowers) << "NumRechargingTimers:" << e->m_recharging_powers.size();
    bs.StorePackedBits(1, e->m_recharging_powers.size());
    for(const CharacterPower *pow : e->m_recharging_powers)
    {
        qCDebug(logPowers) << "  RechargeCountdown:" << pow->m_timer_updated << pow->m_recharge_time;
        bs.StoreBits(1, pow->m_timer_updated);
        if(pow->m_timer_updated)
        {
            storePowerSpec(pow->m_power_info.m_pset_idx, pow->m_index, bs);
            bs.StoreFloat(pow->m_recharge_time);
        }
    }

    // All Owned Inspirations
    int max_cols = cd->m_max_insp_cols;
    int max_rows = cd->m_max_insp_rows;
    int max_insps = max_cols * max_rows;
    qCDebug(logPowers) << "Max Insp Slots:" << max_insps;

    storePackedBitsConditional(bs, 4, max_insps);
    for(int i = 0; i < max_cols; ++i)
    {
        for(int j = 0; j < max_rows; ++j)
        {
            qCDebug(logPowers) << "  Inspiration:" << i << j << cd->m_inspirations.at(i, j).m_has_insp;

            bs.StorePackedBits(3, i); // iCol
            bs.StorePackedBits(3, j); // iRow

            bs.StoreBits(1, cd->m_inspirations.at(i, j).m_has_insp);
            if(cd->m_inspirations.at(i, j).m_has_insp)
                cd->m_inspirations.at(i, j).m_insp_info.serializeto(bs);
        }
    }

    // All Owned Enhancements
    qCDebug(logPowers) << "Enhancement Slots:" << cd->m_enhancements.size();

    bs.StorePackedBits(1, cd->m_enhancements.size());
    for(CharacterEnhancement &eh : cd->m_enhancements)
    {
        if(cd->m_enhancements.empty() || eh.m_name.isEmpty())
        {
            qCDebug(logPowers) << "  No Enhancement:" << eh.m_slot_idx;
            bs.StorePackedBits(3, eh.m_slot_idx);
            bs.StoreBits(1, false);
            continue;
        }

        qCDebug(logPowers) << "  Enhancement:" << eh.m_slot_idx
                           << eh.m_slot_used;
        bs.StorePackedBits(3, eh.m_slot_idx);
        bs.StoreBits(1, eh.m_slot_used);
        if(eh.m_slot_used)
        {
            eh.m_enhance_info.serializeto(bs);
            bs.StorePackedBits(5, eh.m_level);
            bs.StorePackedBits(2, eh.m_num_combines);
        }
    }

    cd->m_powers_updated = false; // reset flag now that we've updated
    qCDebug(logPowers) << "  Powers Updated:" << cd->m_powers_updated;
}

void sendServerControlState(const EntitiesResponse &src,BitStream &bs)
{
    Entity *ent = src.m_client->m_ent; // user entity

    SurfaceParams surface_params[2]; // idx 0 is walking, idx 1 is flying
    memset(&surface_params,0,2*sizeof(SurfaceParams));
    surface_params[0].traction = 1.0f;
    surface_params[0].friction = 0.45f;
    surface_params[0].bounce = 0.01f;
    surface_params[0].gravity = 0.065f;
    surface_params[0].max_speed = 1.00f;
    surface_params[1].traction = 0.02f;
    surface_params[1].friction = 0.01f;
    surface_params[1].bounce = 0.00f;
    surface_params[1].gravity = 0.065f;
    surface_params[1].max_speed = 1.00f;

    bs.StoreBits(1,ent->m_motion_state.m_update_motion_state);
    if(ent->m_motion_state.m_update_motion_state)
    {
        //rand()&0xFF
        bs.StoreBits(8, ent->m_motion_state.m_motion_state_id);
        // after input_send_time_initialized, this value is enqueued as CSC_9's control_flags

        storeVector(bs, ent->m_motion_state.m_speed);           // This is entity speed vector !!

        bs.StoreFloat(ent->m_motion_state.m_backup_spd);        // Backup Speed default = 1.0f
        bs.StoreBitArray((uint8_t *)&surface_params,2*sizeof(SurfaceParams)*8);

        bs.StoreFloat(ent->m_motion_state.m_jump_height);       // How high entity goes before gravity bring them back down. Set by leaping default = 0.1f
        bs.StoreBits(1, ent->m_motion_state.m_is_flying);       // is_flying flag
        bs.StoreBits(1, ent->m_motion_state.m_is_stunned);      // is_stunned flag (lacks overhead 'dizzy' FX)
        bs.StoreBits(1, ent->m_motion_state.m_has_jumppack);    // jumpack flag (lacks costume parts)

        bs.StoreBits(1, ent->m_motion_state.m_controls_disabled);   // if 1/true entity anims stop, can still move, but camera stays. Slipping on ice?
        bs.StoreBits(1, ent->m_motion_state.m_is_jumping);          // leaping? seems like the anim changes slightly?
        bs.StoreBits(1, ent->m_motion_state.m_is_sliding);          // sliding? default = 0
    }

    // Used to force the client to a position/speed/pitch/rotation by server
    bs.StoreBits(1, ent->m_update_pos_and_cam);
    if(ent->m_update_pos_and_cam)
    {
        bs.StorePackedBits(1, ent->m_states.current()->m_every_4_ticks);    // sets g_client_pos_id_rel default = 0
        storeVector(bs, ent->m_entity_data.m_pos);                          // server-side pos
        storeVectorConditional(bs, ent->m_motion_state.m_speed);            // server-side spd

        storeFloatConditional(bs, ent->m_states.current()->m_camera_pyr.x); // Pitch
        storeFloatConditional(bs, ent->m_states.current()->m_camera_pyr.y); // Yaw
        storeFloatConditional(bs, ent->m_states.current()->m_camera_pyr.z); // Roll

        bs.StorePackedBits(1, ent->m_motion_state.m_is_falling);            // server side forced falling bit
        ent->m_update_pos_and_cam = false; // run once
    }
}

void sendServerPhysicsPositions(const EntitiesResponse &src,BitStream &bs)
{
    Entity * target = src.m_client->m_ent;

    bs.StoreBits(1,target->m_full_update);
    if( !target->m_full_update )
        bs.StoreBits(1,target->m_has_control_id);

    qCDebug(logPosition,"Input_Ack: %d", target->m_input_pkt_id);

    if( target->m_full_update || target->m_has_control_id)
        bs.StoreBits(16, target->m_input_pkt_id); //target->m_input_ack

    if(target->m_full_update) // target->m_full_update
    {
        for(int i=0; i<3; ++i)
            bs.StoreFloat(target->m_entity_data.m_pos[i]); // server position
        for(int i=0; i<3; ++i)
            storeFloatConditional(bs, target->m_motion_state.m_velocity[i]);

        qCDebug(logPosition) << "position" << glm::to_string(target->m_entity_data.m_pos).c_str();
        qCDebug(logPosition) << "velocity" << glm::to_string(target->m_motion_state.m_velocity).c_str();
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

void sendClientData(const GameDataStore &data,const EntitiesResponse &src,BitStream &bs)
{
    Entity *ent=src.m_client->m_ent;

    if(!src.m_incremental)
    {
        //full_update - > receiveCharacterFromServer
        // initial character update = level/name/class/origin/map_name
        //m_client->char_entity()->m_char->m_ent=m_client->char_entity();
        serialize_char_full_update(*ent,bs);
    }
    else
    {
        const Character &player_char(*ent->m_char);
        player_char.sendFullStats(bs);
    }
    //qCDebug(logPowers) << "Before storePowerInfoUpdate";
    storePowerInfoUpdate(data,src,bs);
    //qCDebug(logPowers) << "After storePowerInfoUpdate";
    storeTeamList(src,bs);
    storeSuperStats(src,bs);
    storeGroupDyn(src,bs);

    bs.StoreBits(1,ent->m_force_camera_dir);
    if(ent->m_force_camera_dir)
    {
        bs.StoreFloat(ent->m_states.current()->m_camera_pyr.p); // force camera_pitch
        bs.StoreFloat(ent->m_states.current()->m_camera_pyr.y); // force camera_yaw
        bs.StoreFloat(ent->m_states.current()->m_camera_pyr.r); // force camera_roll
    }
}
}

//! EntitiesResponse is sent to a client to inform it about the current world state.
EntitiesResponse::EntitiesResponse(MapClientSession *cl) :
    MapLinkEvent(MapEventTypes::evEntitiesResponse)
{
    m_map_time_of_day       = 10;
    m_client                = {cl ? cl->m_session_token:0,cl};
    g_interpolation_level   = 2;
    g_interpolation_bits    = 1;
}

void EntitiesResponse::serializefrom(BitStream &)
{
    assert(false);
}
void EntitiesResponse::serializeto( BitStream &tgt ) const
{
    MapInstance *mi = m_client->m_current_map;
    EntityManager &ent_manager(mi->m_entities);

    tgt.StorePackedBits(1, m_incremental ? 2 : 3); // opcode  3 - full update.

    tgt.StoreBits(1, ent_major_update); // passed to Entity::EntReceive as a parameter

    sendCommands(*this,tgt);

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
    //else debug_info = false;
    ent_manager.sendEntities(tgt,*m_client,m_incremental);
    if(debug_info)
    {
        ent_manager.sendDebuggedEntities(tgt); // while loop, sending entity id's and debug info for each
        ent_manager.sendGlobalEntDebugInfo(tgt);
    }
    sendServerPhysicsPositions(*this,tgt); // These are not client specific ?
    sendControlState(*this,tgt);// These are not client specific ?
    ent_manager.sendDeletes(tgt,*m_client);
    // Client specific part
    sendClientData(mi->serverData(),*this,tgt);
    // Server messages follow the entity update.
    auto v= std::move(m_client->m_contents);
    for(const auto &command : v)
        command->serializeto(tgt);
    tgt.StorePackedBits(1,0); // finalize the command list
}

//! @}
