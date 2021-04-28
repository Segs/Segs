/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "EntityUpdateCodec.h"
#include "MapClientSession.h"

#include "GameData/Character.h"
#include "GameData/Entity.h"
#include "GameData/EntityHelpers.h"
#include "MapServer.h"
#include "GameData/GameDataStore.h"
#include "GameData/CoHMath.h"
#include "NetFxHelpers.h"
#include "DataHelpers.h"
#include "Components/Logging.h"

#include <glm/ext.hpp> // currently only needed for logOrientation debug
#include <inttypes.h>

namespace  {
void storeCreation(const Entity &src, BitStream &bs)
{
    // entity creation
    bs.StoreBits(1,src.m_destroyed); // ends creation destroys seq and returns NULL

    if(src.m_destroyed)
        return;

    bs.StorePackedBits(12,src.m_idx);//  this will be put in  of created entity
    PUTDEBUG("after id");
    bs.StorePackedBits(2,uint8_t(src.m_type));
    if(src.m_type==EntType::PLAYER)
    {
        bs.StoreBits(1,src.m_create_player);
        if(src.m_create_player)
            bs.StorePackedBits(1,src.m_entity_data.m_access_level);
        bs.StorePackedBits(20,src.m_db_id); // formerly src.m_idx
    }
    else
    {
        bool val=src.m_npc->m_is_owned;
        bs.StoreBits(1,val);
        if(val)
        {
            bs.StorePackedBits(12,src.ownerEntityId); // entity idx
            bs.StorePackedBits(12,src.creatorEntityId); // entity idx
        }
    }
    PUTDEBUG("after creatorowner");
    if(src.m_type==EntType::PLAYER || src.m_type==EntType::CRITTER)
    {
        bs.StorePackedBits(1,src.m_entity_data.m_class_idx);
        bs.StorePackedBits(1,src.m_entity_data.m_origin_idx);
        bs.StoreBits(1, src.m_char->m_char_data.m_has_titles); // Does entity have titles?
        if(src.m_char->m_char_data.m_has_titles)
            src.m_char->sendTitles(bs,NameFlag::NoName,ConditionalFlag::Conditional); // NoName b/c We send it below
    }
    bs.StoreBits(1,src.m_hasname);
    if(src.m_hasname)
        bs.StoreString(src.m_char->getName());
    PUTDEBUG("after names");
    bs.StoreBits(1,src.m_is_fading); // Is entity being faded in ?
    // the following is used as an input to LCG float generator, generated float (0-1) is used as
    // linear interpolation factor between scale_min and scale_max
    bs.StoreBits(32, src.m_randSeed);
    bs.StoreBits(1, src.m_faction_data.m_has_faction);
    if(src.m_faction_data.m_has_faction)
    {
        bs.StorePackedBits(2, src.m_faction_data.m_rank);    // this will be put in field_1830 (iRank) of created entity
        bs.StoreString(src.m_faction_data.m_faction_name);  // villain group name?
    }
    PUTDEBUG("end storeCreation");
}

void sendStateMode(const Entity &src, BitStream &bs)
{
    // if(state_mode & 2) then RespawnIfDead, AliveEnough==true, close some windows
    PUTDEBUG("before sendStateMode");
    bs.StoreBits(1, src.m_entity_update_flags.testFlag(src.UpdateFlag::STATEMODE));
    PUTDEBUG("before sendStateMode 2");
    if(src.m_entity_update_flags.testFlag(src.UpdateFlag::STATEMODE))
        storePackedBitsConditional(bs, 3, uint32_t(src.m_state_mode));

    PUTDEBUG("after sendStateMode");
}

void storeInterpolationTree(const Entity &/*src*/,BitStream &bs)
{
    bs.StoreBits(1,0);
}

bool storePosition(const Entity &src,BitStream &bs)
{
// float x = pos.vals.x;
    uint8_t updated_bit_pos = 7; // FixMe: updated_bit_pos is explicitly assigned and never modified later.

    bs.StoreBits(3,updated_bit_pos);

    if(updated_bit_pos==0)
        return false; // no actual update takes place

    for(int i=0; i<3; i++)
    {
        FixedPointValue fpv(src.m_entity_data.m_pos[i]);
        //diff = packed ^ prev_pos[i]; // changed bits are '1'
        bs.StoreBits(24,fpv.store);
    }
    return true;
}

bool update_rot(const Entity &/*src*/, int axis ) /* returns true if given axis needs updating */
{
    int axis_tmp = axis; // Used to quell tautological comparison warning
    if(axis==axis_tmp)   // FixMe: var compared against same var.
        return true;
    return false;
}

void storeOrientation(const Entity &src,BitStream &bs)
{
    // Check if update needed through update_rot()
    uint8_t updates;
    updates = ((uint8_t)update_rot(src,0)) | (((uint8_t)update_rot(src,1))<<1) | (((uint8_t)update_rot(src,2))<<2);
    storeBitsConditional(bs,3,updates); //frank 7,0,0.1,0

    qCDebug(logOrientation, "updates: %i",updates);
    glm::vec3 pyr_angles(0);
    pyr_angles.y = src.m_entity_data.m_orientation_pyr.y;
    // output everything
    qCDebug(logOrientation, "Player: %d", src.m_idx);
    qCDebug(logOrientation, "dir: %s", glm::to_string(src.m_direction).c_str());
    qCDebug(logOrientation, "pyr_angles: farr(%f, %f, %f)", pyr_angles[0], pyr_angles[1], pyr_angles[2]);
    qCDebug(logOrientation, "orient_p: %f", src.m_entity_data.m_orientation_pyr[0]);
    qCDebug(logOrientation, "orient_y: %f", src.m_entity_data.m_orientation_pyr[1]);

    for(int i=0; i<3; i++)
    {
        if(!update_rot(src,i))
            continue;

        uint32_t v = AngleQuantize(pyr_angles[i],9);
        qCDebug(logOrientation, "v: %d", v); // does `v` fall between 0...512
        bs.StoreBits(9,v);
    }
}

void storePosUpdate(const Entity &src, bool just_created, BitStream &bs)
{
    PUTDEBUG("before entReceivePosUpdate");
    bool position_updated = storePosition(src, bs);

    PUTDEBUG("before posInterpolators");
    if(!just_created && position_updated)
    {
        // if position has changed
        // prepare interpolation table, given previous position
        bs.StoreBits(1, src.m_has_interp);
        if(src.m_has_interp)
        {
            bs.StoreBits(1, src.m_move_instantly);
            if(!src.m_move_instantly)
                storeInterpolationTree(src, bs); // Bintree sending happens here
        }
    }
    PUTDEBUG("before storeOrientation");
    storeOrientation(src,bs);
    PUTDEBUG("after storeOrientation");
}

void sendSeqMoveUpdate(const Entity &src, BitStream &bs)
{
    if(src.m_type == EntType::PLAYER)
        qCDebug(logAnimations, "Sending seq mode update %d", src.m_seq_update);

    PUTDEBUG("before sendSeqMoveUpdate");
    bs.StoreBits(1, src.m_seq_update); // no seq update
    if(src.m_seq_update)
    {
        storePackedBitsConditional(bs, 8, src.m_seq_move_idx); // move index
        storePackedBitsConditional(bs, 4, src.m_seq_move_change_time); // maxval is 255
    }
}
void sendSeqTriggeredMoves(const Entity &src,BitStream &bs)
{
    PUTDEBUG("before sendSeqTriggeredMoves");
    if(src.m_type == EntType::PLAYER)
        qCDebug(logAnimations, "Sending seq triggered moves %" PRIu64, src.m_triggered_moves.size());

    // client appears to process only the last 20 triggered moves
    bs.StorePackedBits(1, src.m_triggered_moves.size()); // num moves
    for(const TriggeredMove &move : src.m_triggered_moves)
    {
        bs.StorePackedBits(10, move.m_move_idx);                   // 2  triggeredMoveIDX
        bs.StorePackedBits(6, move.m_ticks_to_delay);              // 0  ticksToDelay
        storePackedBitsConditional(bs, 16, move.m_trigger_fx_idx); // 1 triggerFxNetId
    }
}

void sendNetFx(const Entity &src, BitStream &bs)
{
    bs.StorePackedBits(1, src.m_net_fx.size()); // num fx
    for(NetFxHandle fxh : src.m_net_fx)
    {
        const NetFx &fx(lookup(fxh));
        // refactor as fx.serializeto() ?
        bs.StoreBits(8, fx.command); // command
        bs.StoreBits(32, fx.net_id); // NetID
        bs.StoreBits(1, fx.pitch_to_target);
        storePackedBitsConditional(bs, 10, fx.handle); // handle
        storeBitsConditional(bs, 4, fx.client_timer); // client timer
        storeBitsConditional(bs, 32, fx.client_trigger_fx); // clientTriggerFx
        storeFloatConditional(bs, fx.duration); // duration
        storeFloatConditional(bs, fx.radius); // radius
        storeBitsConditional(bs, 4, fx.power);  // power
        storeBitsConditional(bs, 32, fx.debris);  // debris

        storeBitsConditional(bs, 2, fx.origin.type_is_location); // origiType
        if(fx.origin.type_is_location)
        {
            bs.StoreFloat(fx.origin.pos.x); // origin Pos
            bs.StoreFloat(fx.origin.pos.y);
            bs.StoreFloat(fx.origin.pos.z);
        }
        else
        {
            storePackedBitsConditional(bs, 8, fx.origin.ent_idx); // origin entity
            bs.StorePackedBits(2,fx.origin.bone_idx); // bone id
        }

        storeBitsConditional(bs, 2, fx.target.type_is_location); // target type
        if(fx.target.type_is_location)
        {
            bs.StoreFloat(fx.target.pos.x); // targetPos x
            bs.StoreFloat(fx.target.pos.y); // targetPos y
            bs.StoreFloat(fx.target.pos.z); // targetPos z
        }
        else
            storePackedBitsConditional(bs, 12, fx.target.ent_idx); // target entity
    }
}

void sendCostumes(const Entity &src,BitStream &bs)
{
    //NOTE: this will only be initialized once, and no changes later on will influence this
    static const ColorAndPartPacker *packer = getGameData().getPacker();
    PUTDEBUG("before sendCostumes");
    storePackedBitsConditional(bs,2,uint8_t(src.m_costume_type));
    switch(src.m_costume_type)
    {
        case AppearanceType::WholeCostume: // client value 1
            src.m_char->serialize_costumes(bs,packer,true); // we're always sending full info
            break;
        case AppearanceType::NpcCostume: // client value 2 top level defs from VillainCostume ?
            bs.StorePackedBits(12,src.m_npc->npc_idx); // npc costume type idx ?
            bs.StorePackedBits(1,src.m_npc->costume_variant); // npc costume idx ?
            break;
        case AppearanceType::SequencerName: // client val 4
            bs.StoreString("Unknown"); // this is mostly used to send mcguffins :)
            break;
        default:
            assert(false);
            break;
    }
}

void sendXLuency(BitStream &bs,float val)
{
    storeBitsConditional(bs,8,std::min(static_cast<int>(uint8_t(val*255)),255));
}

void sendCharacterStats(const Entity &src,BitStream &bs)
{
    // FixMe: have_stats and stats_changed are never modified prior to if comparison below.
    bool have_stats = true; // no stats -> dead ?
    bool stats_changed = true;

    bs.StoreBits(1,have_stats); // nothing here for now
    if(!have_stats)
        return;
    bs.StoreBits(1,stats_changed);
    if(!stats_changed)
        return;

    // Store Sidekick Info
    bs.StoreBits(1,src.m_char->m_char_data.m_sidekick.m_has_sidekick);
    if(src.m_char->m_char_data.m_sidekick.m_has_sidekick)
    {
        Sidekick sidekick = src.m_char->m_char_data.m_sidekick;
        bool is_mentor = isSidekickMentor(src);
        bool has_dbid  = (sidekick.m_db_id != 0);

        bs.StoreBits(1,is_mentor);
        bs.StoreBits(1, has_dbid);
        if(has_dbid)
            bs.StorePackedBits(20,sidekick.m_db_id);
    }

    serializeStats(*src.m_char,bs,false);
}

void sendBuffsConditional(const Entity &src, BitStream &bs)
{
    bs.StoreBits(1, src.m_update_buffs);
    if(src.m_update_buffs)
        sendBuffs(src,bs);
}

void sendTargetUpdate(const Entity &src,BitStream &bs)
{
    uint32_t assist_id  = getAssistTargetIdx(src);
    uint32_t target_id  = getTargetIdx(src);
    bool has_target     = (target_id != 0);

    bs.StoreBits(1,has_target);
    if(!has_target)
        return;

    bs.StoreBits(1,target_id!=0);
    if(target_id!=0)
        bs.StorePackedBits(12,target_id);
    bs.StoreBits(1,assist_id!=0);
    if(assist_id!=0)
        bs.StorePackedBits(12,assist_id);
}

void sendOnOddSend(const Entity &src,BitStream &bs)
{
    // if this is set the entity on client will :
    // set move change timer to be always 0
    // calculate interpolations using slow timer
    //
    bs.StoreBits(1, src.m_entity_update_flags.testFlag(src.UpdateFlag::ODDSEND));
}

void sendWhichSideOfTheForce(const Entity &src,BitStream &bs)
{
    bs.StoreBits(1,src.m_is_villain); // on team evil ?
    bs.StoreBits(1,src.m_is_hero); // on team good ?
}
void sendEntCollision(const Entity &src,BitStream &bs)
{
    // if 1 is sent, client will disregard it's own collision processing.
    bs.StoreBits(1, src.m_motion_state.m_no_collision); // 1/0 only
}

void sendNoDrawOnClient(const Entity &src,BitStream &bs)
{
    bs.StoreBits(1, src.m_entity_update_flags.testFlag(src.UpdateFlag::NODRAWONCLIENT)); // 1/0 only
}

void sendAFK(const Entity &src, BitStream &bs)
{
    const CharacterData &cd(src.m_char->m_char_data);
    bool hasMsg = !cd.m_afk_msg.isEmpty();
    bs.StoreBits(1, cd.m_afk); // 1/0 only
    if(cd.m_afk)
    {
        bs.StoreBits(1,hasMsg); // 1/0 only, 1 = has afk msg
        if(hasMsg)
            bs.StoreString(cd.m_afk_msg);
    }
}

void sendOtherSupergroupInfo(const Entity &src,BitStream &bs)
{
    bs.StoreBits(1,src.m_has_supergroup); // src.m_has_supergroup?
    if(!src.m_has_supergroup)
        return;
    bs.StorePackedBits(2,src.m_supergroup.m_SG_id);
    if(src.m_supergroup.m_SG_id)
    {
        bs.StoreString(src.m_supergroup.m_SG_name);//64 chars max
        bs.StoreString("");//128 chars max -> hash table key from the CostumeString_HTable. Maybe emblem?
        bs.StoreBits(32,src.m_supergroup.m_SG_color1); // supergroup color 1
        bs.StoreBits(32,src.m_supergroup.m_SG_color2); // supergroup color 2
    }
}

void sendLogoutUpdate(const Entity &src,ClientEntityStateBelief &belief,BitStream &bs)
{
    if(belief.m_is_logging_out == src.m_is_logging_out) // no change in logout state
    {
        bs.StoreBits(1, false);
        return;
    }
    bs.StoreBits(1, true); // logout state update
    bs.StoreBits(1, 0); // if 1 then it means the logout was caused by low connection quality.
    // we send 0 as a time to logout if this is a logout-abort
    storePackedBitsConditional(bs, 5, src.m_is_logging_out ? src.m_time_till_logout/(1000) : 0);
    belief.m_is_logging_out = src.m_is_logging_out;
}
////////////////////////////////////////////////////////////////////////////////////////////
} // end of anonoymous namespace
void sendBuffs(const Entity &src, BitStream &bs)
{
    bs.StorePackedBits(5, src.m_buffs.size());
    for(const Buffs &buff : src.m_buffs)
        buff.m_buff_info.serializeto(bs);
}

void serializeto(const Entity & src, ClientEntityStateBelief &belief, BitStream &bs )
{
    bool client_believes_ent_exists=belief.m_entity!=nullptr;
    bool ent_exists = src.m_destroyed==false;
    bool update_existence=client_believes_ent_exists!=ent_exists;

    //////////////////////////////////////////////////////////////////////////
    bs.StoreBits(1,update_existence);
    if(update_existence)
        storeCreation(src,bs);
    belief.m_entity = ent_exists ? &src : nullptr;
    if(!ent_exists)
        return;
    //////////////////////////////////////////////////////////////////////////
    // creation ends here
    PUTDEBUG("before entReceiveStateMode");

    bool update_rarely = src.m_entity_update_flags & (src.UpdateFlag::FULL
            | src.UpdateFlag::LOGOUT | src.UpdateFlag::SUPERGROUP | src.UpdateFlag::AFK
            | src.UpdateFlag::NODRAWONCLIENT | src.UpdateFlag::NOCOLLISION | src.UpdateFlag::HEROVILLAIN
            | src.UpdateFlag::ODDSEND | src.UpdateFlag::TITLES | src.UpdateFlag::TRANSLUCENCY
            | src.UpdateFlag::COSTUMES | src.UpdateFlag::ANIMATIONS | src.UpdateFlag::STATEMODE);
    bool update_chars = src.m_entity_update_flags & (src.UpdateFlag::TARGET
            | src.UpdateFlag::BUFFS | src.UpdateFlag::STATS | src.UpdateFlag::FX);
    bool has_updates = src.m_entity_update_flags;

    // NPCs never have pchar_things (FX, Stats, Buffs, Targets)
    // Critters and Players may or may not depending on state
    if(src.m_type == EntType::NPC || src.m_type == EntType::DOOR)
        update_chars = false;

    bs.StoreBits(1, has_updates);
    if(has_updates)
        bs.StoreBits(1, update_rarely);

    if(update_rarely)
        sendStateMode(src, bs);

    storePosUpdate(src, update_existence && ent_exists, bs);

    if(has_updates)
        sendSeqMoveUpdate(src, bs);

    if(update_rarely)
        sendSeqTriggeredMoves(src, bs);

    PUTDEBUG("before m_pchar_things");
    bs.StoreBits(1, update_chars);
    if(update_chars)
        sendNetFx(src,bs);

    if(update_rarely)
    {
        sendCostumes(src, bs);
        sendXLuency(bs, src.translucency);
        bs.StoreBits(1, src.m_char->m_char_data.m_has_titles); // Does entity have titles?
        if(src.m_char->m_char_data.m_has_titles)
            src.m_char->sendTitles(bs, NameFlag::HasName, ConditionalFlag::Conditional);
    }

    if(update_chars)
    {
        sendCharacterStats(src, bs);
        sendBuffsConditional(src, bs);
        sendTargetUpdate(src, bs);
    }

    if(update_rarely)
    {
        sendOnOddSend(src, bs); // is one on client end
        sendWhichSideOfTheForce(src, bs);
        sendEntCollision(src, bs);
        sendNoDrawOnClient(src, bs);
        sendAFK(src, bs);
        sendOtherSupergroupInfo(src, bs);
        sendLogoutUpdate(src, belief, bs);
    }
}

//! @}
