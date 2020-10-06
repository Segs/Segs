/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/
 * @{
 */

#include "MessageHelpers.h"

#include "DataHelpers.h"
#include "EntityStorage.h"
#include "EntityUpdateCodec.h"
#include "MapClientSession.h"
#include "MapInstance.h"
#include "Common/GameData/CommonNetStructures.h"
#include "Common/GameData/CharacterHelpers.h"
#include "Common/GameData/Movement.h"
#include "Common/GameData/map_definitions.h"
#include "Common/GameData/playerdata_definitions.h"
#include "Messages/Map/ChatMessage.h"
#include "Messages/Map/InfoMessageCmd.h"
#include "Messages/Map/EntitiesResponse.h"
#include "Common/GameData/Character.h"
#include "Components/Logging.h"

#include <glm/gtx/string_cast.hpp>


using namespace SEGSEvents;

void sendChatMessage(MessageChannel t, const QString &msg, Entity *e, MapClientSession &tgt)
{
    std::unique_ptr<ChatMessage> res = std::make_unique<ChatMessage>(t,msg);
    res->m_source_player_id = getIdx(*e);
    res->m_target_player_id = getIdx(*tgt.m_ent);

    qCDebug(logChat).noquote() << "ChatMessage:"
                               << "\n  Channel:" << int(res->m_channel_type)
                               << "\n  Source:" << res->m_source_player_id
                               << "\n  Target:" << res->m_target_player_id
                               << "\n  Message:" << res->m_msg;

    tgt.addCommandToSendNextUpdate(std::move(res));
}

void sendInfoMessage(MessageChannel t, const QString &msg, MapClientSession &tgt)
{
    tgt.addCommand<InfoMessageCmd>(t, msg);

    qCDebug(logInfoMsg).noquote() << "InfoMessage:"
             << "\n  Channel:" << static_cast<int>(t)
             << "\n  Message:" << msg;
}

void storeEntityResponseCommands(BitStream &bs,float time_of_day)
{
    PUTDEBUG("before commands");
    bs.StorePackedBits(1,1); // use 'time' shortcut
    bs.StoreFloat(time_of_day*10.0f);
    bs.StorePackedBits(1,2); // use 'time scale' shortcut
    bs.StoreFloat(4.0f);
    bs.StorePackedBits(1,3); // use 'time step scale' shortcut
    bs.StoreFloat(2.0f);
    bs.StorePackedBits(1,4); // use 'pause' shortcut
    bs.StorePackedBits(1,0); // if `true` no calcs happen
    bs.StorePackedBits(1,5); // use 'disablegurneys' shortcut
    bs.StorePackedBits(1,0); // if `false` spawn instantly
    bs.StorePackedBits(1,0);
    PUTDEBUG("after commands");
}

void storeEntityResponseOtherEntities(BitStream &tgt,EntityManager &manager, MapClientSession *self, bool incremental,bool debug)
{
    manager.sendEntities(tgt,*self,incremental);
    if(debug)
    {
        manager.sendDebuggedEntities(tgt); // while loop, sending entity id's and debug info for each
        manager.sendGlobalEntDebugInfo(tgt);
    }
}

void storeServerPhysicsPositions(BitStream &bs,Entity *self)
{
    PUTDEBUG("before physics");

    bs.StoreBits(1,self->m_full_update);
    if( !self->m_full_update )
        bs.StoreBits(1,self->m_has_control_id);

    qCDebug(logPosition,"Input_Ack: %d",self->m_input_ack);

    if( self->m_full_update || self->m_has_control_id)
        bs.StoreBits(16,self->m_input_ack); //target->m_input_ack
    if(self->m_full_update)
    {
        for(int i=0; i<3; ++i)
            bs.StoreFloat(self->m_entity_data.m_pos[i]); // server position
        for(int i=0; i<3; ++i)
            storeFloatConditional(bs,self->m_motion_state.m_velocity[i]);

        qCDebug(logPosition) << "position" << glm::to_string(self->m_entity_data.m_pos).c_str();
        qCDebug(logPosition) << "velocity" << glm::to_string(self->m_motion_state.m_velocity).c_str();
    }
}

void storeServerControlState(BitStream &bs,Entity *self)
{
    bs.StoreBits(1,self->m_update_part_1);
    if(self->m_update_part_1)
    {
        //rand()&0xFF
        bs.StoreBits(8,self->m_update_id);
        // after input_send_time_initialized, this value is enqueued as CSC_9's control_flags

        storeVector(bs,self->m_motion_state.m_speed); // This is entity speed vector !!

        bs.StoreFloat(self->m_motion_state.m_backup_spd);         // Backup Speed default = 1.0f
        bs.StoreBitArray((uint8_t *)&self->m_motion_state.m_surf_mods,2*sizeof(SurfaceParams)*8);

        bs.StoreFloat(self->m_motion_state.m_jump_height);        // How high entity goes before gravity bring them back down. Set by leaping default = 0.1f
        bs.StoreBits(1,self->m_motion_state.m_is_flying);         // is_flying flag
        bs.StoreBits(1,self->m_motion_state.m_is_stunned);        // is_stunned flag (lacks overhead 'dizzy' FX)
        bs.StoreBits(1,self->m_motion_state.m_has_jumppack);      // jumpack flag (lacks costume parts)

        bs.StoreBits(1,self->m_motion_state.m_controls_disabled); // if 1/true entity anims stop, can still move, but camera stays. Slipping on ice?
        bs.StoreBits(1,self->m_motion_state.m_is_jumping);        // leaping? seems like the anim changes slightly?
        bs.StoreBits(1,self->m_motion_state.m_is_sliding);        // sliding? default = 0
    }
    // Used to force the client to a position/speed/pitch/rotation by server
    bs.StoreBits(1,self->m_force_pos_and_cam);
    if(self->m_force_pos_and_cam)
    {
        bs.StorePackedBits(1,self->m_input_state.m_every_4_ticks);               // sets g_client_pos_id_rel default = 0
        storeVector(bs,self->m_entity_data.m_pos);                          // server-side pos
        storeVectorConditional(bs,self->m_motion_state.m_velocity);         // server-side velocity

        storeFloatConditional(bs,self->m_entity_data.m_orientation_pyr.x);  // Pitch not used ?
        storeFloatConditional(bs,self->m_entity_data.m_orientation_pyr.y);  // Yaw
        storeFloatConditional(bs,self->m_entity_data.m_orientation_pyr.z);  // Roll
        bs.StorePackedBits(1,self->m_motion_state.m_is_falling);            // server side forced falling bit

        self->m_force_pos_and_cam = false; // run once
    }
}

void storeControlState(BitStream &bs,Entity *self)
{
    PUTDEBUG("before control state");
    storeServerControlState(bs,self);
}

void storeEntityRemovals(BitStream &bs, EntityManager &manager,MapClientSession *self)
{
    PUTDEBUG("before removals");
    manager.sendDeletes(bs,*self);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace
{


void storeSuperStats(BitStream &bs)
{
    bs.StorePackedBits(1,0);
}

void storeGroupDyn(BitStream &bs)
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

void storeTeamList(BitStream &bs, Entity *self)
{
    assert(self);

    // shorthand local vars
    uint32_t    team_idx = 0;
    bool        mark_lfg = self->m_char->m_char_data.m_lfg;
    bool        has_taskforce = false;
    uint32_t    tm_leader_id = 0;
    uint32_t    tm_size = 0;

    if(self->m_has_team && self->m_team != nullptr)
    {
        team_idx        = self->m_team->m_data.m_team_idx;
        has_taskforce   = self->m_team->m_data.m_has_taskforce;
        tm_leader_id    = self->m_team->m_data.m_team_leader_idx;
        tm_size         = self->m_team->m_data.m_team_members.size();
    }

    storePackedBitsConditional(bs,20,team_idx);
    bs.StoreBits(1,has_taskforce);
    bs.StoreBits(1,mark_lfg);

    if(team_idx == 0) // if no team, return.
        return;

    bs.StoreBits(32,tm_leader_id); // must be db_id
    bs.StorePackedBits(1,tm_size);

    for(const auto &member : self->m_team->m_data.m_team_members)
    {
        Entity *tm_ent = getEntityByDBID(self->m_client->m_current_map, member.tm_idx);

        if(tm_ent == nullptr)
        {
            qWarning() << "Could not find Entity with db_id:" << member.tm_idx;
            continue; // continue loop
        }
        if(member.tm_name.isEmpty())
        {
            qWarning() << "Team member with empty name:" << member.tm_idx;
            continue; // continue loop
        }

        QString member_name     = member.tm_name;
        QString member_mapname  = getEntityDisplayMapName(tm_ent->m_entity_data);
        bool tm_on_same_map     = member.tm_map_idx == self->m_entity_data.m_map_idx;

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
            bs.StoreFloat(p.getPowerTemplate().Range); // nem: I have no idea why it is passed here
        }
    }
    bs.StoreBits(1,0); // no more powers to send.
    //qCDebug(logPowers) << "No more powers to send.";
}

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

void storePowerInfoUpdate(BitStream &bs,Entity *e)
{
    assert(e);
    CharacterData *cd = &e->m_char->m_char_data;

    bs.StoreBits(1, cd->m_has_updated_powers);
    if(!cd->m_has_updated_powers)
    {
        storePowerRanges(*cd, bs);
        return;
    }
    qCDebug(logPowers) << "Powers Updated:" << cd->m_has_updated_powers;

    // Reset Powersets (the array has changed)
    bs.StoreBits(1, cd->m_reset_powersets);
    if(cd->m_reset_powersets)
        cd->m_reset_powersets = false; // toggle this false because we're done

    bs.StorePackedBits(1, countAllOwnedPowers(*cd, true)); // must include temp powers
    uint pset_idx = 0;
    for(const CharacterPowerSet &pset : cd->m_powersets)
    {
        uint pow_idx = 0;
        for(const CharacterPower &power : pset.m_powers)
        {
            qCDebug(logPowers) << "Power:" << power.getPowerTemplate().m_Name << pset.m_index << power.m_index;
            storePowerSpec(pset_idx, pow_idx, bs); // must send powers vector idx here

            bs.StoreBits(1, !power.m_erase_power);
            if(power.m_erase_power)
            {
                qCDebug(logPowers) << "  Removing power" << power.getPowerTemplate().m_Name << pset.m_index << power.m_index;
                removePower(*cd, power.m_power_info);
                continue;
            }

            power.m_power_info.serializeto(bs);
            bs.StorePackedBits(5, pset.m_level_bought);
            bs.StorePackedBits(5, power.m_level_bought);
            bs.StorePackedBits(3, power.m_charges_remaining);
            bs.StoreFloat(power.m_usage_time);
            bs.StorePackedBits(24, power.m_activate_period);

            qCDebug(logPowers) << "  NumOfEnhancements:" << power.m_total_eh_slots << "/" << power.m_enhancements.size();
            if(power.m_total_eh_slots > power.m_enhancements.size())
                qCWarning(logPowers) << "storePowerInfoUpdate: Total EH Slots larger than vector!";

            bs.StorePackedBits(4, power.m_enhancements.size()); // power.m_total_eh_slots; total owned enhancement slots
            for(const auto & enhancement : power.m_enhancements)
            {
                qCDebug(logPowers) << "  Enhancement:" << enhancement.m_name
                                   << enhancement.m_slot_idx
                                   << enhancement.m_slot_used;
                bs.StoreBits(1, enhancement.m_slot_used);        // slot has enhancement
                if(enhancement.m_slot_used)
                {
                    enhancement.m_enhance_info.serializeto(bs);
                    bs.StorePackedBits(5, enhancement.m_level);
                    bs.StorePackedBits(2, enhancement.m_num_combines);
                }
            }
            ++pow_idx;
        }
        ++pset_idx;
    }

    qCDebug(logPowers) << "Send State of All Powers";
    storePowerRanges(*cd, bs); // sending state of all current powers.

    qCDebug(logPowers) << "NumQueuedPowers:" << e->m_queued_powers.size();
    bs.StorePackedBits(4, uint32_t(e->m_queued_powers.size())); // Count all active powers
    for(auto rpow_idx = e->m_queued_powers.begin(); rpow_idx != e->m_queued_powers.end();)
    {
        qCDebug(logPowers) << "  QueuedPower:"
                           << rpow_idx->m_pow_idxs.m_pset_vec_idx
                           << rpow_idx->m_pow_idxs.m_pow_vec_idx;

        bs.StoreBits(1, rpow_idx->m_active_state_change);
        if(rpow_idx->m_active_state_change)
        {
            storePowerSpec(rpow_idx->m_pow_idxs.m_pset_vec_idx, rpow_idx->m_pow_idxs.m_pow_vec_idx, bs);
            bs.StorePackedBits(1, rpow_idx->m_activation_state);
            if (rpow_idx->m_activation_state == false)
                rpow_idx = e->m_queued_powers.erase(rpow_idx);
            else
            {
                rpow_idx->m_active_state_change = false;
                rpow_idx++;
            }
        }
        else
            rpow_idx++;
    }
    qCDebug(logPowers) << "NumRechargingTimers:" << e->m_recharging_powers.size();
    bs.StorePackedBits(1, e->m_recharging_powers.size());
    for(const QueuedPowers &rpow : e->m_recharging_powers)
    {
        qCDebug(logPowers) << "  RechargeCountdown:" << rpow.m_timer_updated << rpow.m_recharge_time;
        bs.StoreBits(1, rpow.m_timer_updated);
        if(rpow.m_timer_updated)
        {
            storePowerSpec(rpow.m_pow_idxs.m_pset_vec_idx, rpow.m_pow_idxs.m_pow_vec_idx, bs);
            bs.StoreFloat(rpow.m_recharge_time);
        }
    }

    // All Owned Inspirations
    uint32_t max_cols = cd->m_max_insp_cols;
    uint32_t max_rows = cd->m_max_insp_rows;
    uint32_t max_insps = max_cols * max_rows;
    qCDebug(logPowers) << "Max Insp Slots:" << max_insps;

    storePackedBitsConditional(bs, 4, max_insps);
    for(uint32_t col = 0; col < max_cols; ++col)
    {
        for(uint32_t row = 0; row < max_rows; ++row)
        {
            qCDebug(logPowers) << "  Inspiration:" << col << row << cd->m_inspirations.at(col, row).m_has_insp;

            bs.StorePackedBits(3, col); // iCol
            bs.StorePackedBits(3, row); // iRow

            bs.StoreBits(1, cd->m_inspirations.at(col, row).m_has_insp);
            if(cd->m_inspirations.at(col, row).m_has_insp)
                cd->m_inspirations.at(col, row).m_insp_info.serializeto(bs);
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

    cd->m_has_updated_powers = false; // set flag false now that we're all updated
    qCDebug(logPowers) << "  Powers Updated:" << cd->m_has_updated_powers;
}

} // end of anonymous namespace

void storeClientData(BitStream &bs,Entity *ent,bool incremental)
{
    PUTDEBUG("Before character data");
    if(!incremental)
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
    storePowerInfoUpdate(bs,ent);
    //qCDebug(logPowers) << "After storePowerInfoUpdate";
    storeTeamList(bs,ent);
    storeSuperStats(bs);
    storeGroupDyn(bs);

    bs.StoreBits(1,ent->m_force_camera_dir);
    if(ent->m_force_camera_dir)
    {
        bs.StoreFloat(ent->m_entity_data.m_orientation_pyr.x); // force camera_pitch
        bs.StoreFloat(ent->m_entity_data.m_orientation_pyr.y); // force camera_yaw
        bs.StoreFloat(ent->m_entity_data.m_orientation_pyr.z); // force camera_roll
    }
    PUTDEBUG("After character data");
}

void storeFollowupCommands(BitStream &bs,MapClientSession *m_client)
{
    // Server messages follow the entity update.
    auto v= std::move(m_client->m_contents);
    for(const auto &command : v)
        command->serializeto(bs);
    bs.StorePackedBits(1,0); // finalize the command list
}

void buildEntityResponse(EntitiesResponse *res,MapClientSession &to_client,EntityUpdateMode mode,bool use_debug)
{
    bool is_incremental = mode!=EntityUpdateMode::FULL;
    res->blob_of_death.StorePackedBits(1,is_incremental ? 2 : 3); // opcode  3 - full update.
    res->blob_of_death.StoreBits(1,res->ent_major_update); // passed to Entity::EntReceive as a parameter
    res->is_incremental(is_incremental); // full world update = op 3

    res->debug_info = use_debug;
    storeEntityResponseCommands(res->blob_of_death,res->m_map_time_of_day);

    res->blob_of_death.StoreBits(32,res->abs_time);
    //tgt.StoreBits(32,db_time);
    bool all_defaults = (use_debug==0) && (res->g_interpolation_level==2) && (res->g_interpolation_bits==1);
    res->blob_of_death.StoreBits(1,all_defaults);
    if(!all_defaults)
    {
        res->blob_of_death.StoreBits(1,use_debug);
        res->blob_of_death.StoreBits(1,res->g_interpolation_level!=0);
        if(res->g_interpolation_level!=0)
        {
            res->blob_of_death.StoreBits(2,res->g_interpolation_level);
            res->blob_of_death.StoreBits(2,res->g_interpolation_bits);
        }
    }

    storeEntityResponseOtherEntities(res->blob_of_death,to_client.m_current_map->m_entities, &to_client, is_incremental,use_debug);
    storeServerPhysicsPositions(res->blob_of_death,to_client.m_ent);
    storeControlState(res->blob_of_death,to_client.m_ent);
    storeEntityRemovals(res->blob_of_death,to_client.m_current_map->m_entities,&to_client);
    storeClientData(res->blob_of_death,to_client.m_ent,is_incremental);
    storeFollowupCommands(res->blob_of_death,&to_client);
}

//! @}
