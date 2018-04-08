#include "EntityUpdateCodec.h"

#include "Character.h"
#include "MapServer.h"
#include "MapServerData.h"
#include "MapClient.h"
#include "Entity.h"
#include "GameData/CoHMath.h"

//#define DEBUG_INPUT
//#define DEBUG_ORIENTATION
#ifdef DEBUG_ORIENTATION
#include <glm/ext.hpp> // currently only needed for DEBUG_ORIENTATION
#endif

namespace  {
void storeCreation(const Entity &src, BitStream &bs)
{
    // entity creation
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\tSending create entity\n")));

    bs.StoreBits(1,src.m_destroyed); // ends creation destroys seq and returns NULL

    if(src.m_destroyed)
        return;

    bs.StorePackedBits(12,src.m_idx);//  this will be put in  of created entity
    PUTDEBUG("after id");
    bs.StorePackedBits(2,src.m_type);
    if(src.m_type==Entity::ENT_PLAYER)
    {
        bs.StoreBits(1,src.m_create_player);
        if(src.m_create_player)
            bs.StorePackedBits(1,src.m_entity_data.m_access_level);
        bs.StorePackedBits(20,src.m_db_id); // formerly src.m_idx
    }
    else
    {
        bool val=false;
        bs.StoreBits(1,val);
        if(val)
        {
            bs.StorePackedBits(12,src.ownerEntityId); // entity idx
            bs.StorePackedBits(12,src.creatorEntityId); // entity idx
        }
    }
    PUTDEBUG("after creatorowner");
    if(src.m_type==Entity::ENT_PLAYER || src.m_type==Entity::ENT_CRITTER)
    {
        bs.StorePackedBits(1,src.m_entity_data.m_class_idx);
        bs.StorePackedBits(1,src.m_entity_data.m_origin_idx);
        src.m_char->sendTitles(bs,NameFlag::NoName,ConditionalFlag::Conditional); // NoName b/c We send it below
    }
    bs.StoreBits(1,src.m_hasname);
    if(src.m_hasname)
        bs.StoreString(src.m_char->getName());
    PUTDEBUG("after names");
    bool fadin = true;
    bs.StoreBits(1,fadin); // Is entity being faded in ?
    // the following is used as an input to LCG float generator, generated float (0-1) is used as
    // linear interpolation factor betwwen scale_min and scale_max
    bs.StoreBits(32,src.m_randSeed);
    bs.StoreBits(1,src.m_team.m_has_team); // could be supergroup?
    if(src.m_team.m_has_team)
    {
        bs.StorePackedBits(2,src.m_team.m_team_rank); // this will be put in field_1830 of created entity
        bs.StoreString(src.m_team.m_team_name);
    }
    PUTDEBUG("end storeCreation");
}
void sendStateMode(const Entity &src,BitStream &bs)
{
    PUTDEBUG("before sendStateMode");
    bs.StoreBits(1,src.m_state_mode_send); // no state mode
    PUTDEBUG("before sendStateMode 2");
    if(src.m_state_mode_send)
    {
        storePackedBitsConditional(bs,3,src.m_state_mode);
    }
    PUTDEBUG("after sendStateMode");
}
struct BinTreeEntry {
    uint8_t x,y,z,d;
};
struct BinTreeBase {
    BinTreeEntry arr[7];
};
void storeUnknownBinTree(const Entity &/*src*/,BitStream &bs)
{
    bs.StoreBits(1,0);
}
bool storePosition(const Entity &src,BitStream &bs)
{
// float x = pos.vals.x;
    uint8_t updated_bit_pos = 7;

    bs.StoreBits(3,updated_bit_pos);

    if(updated_bit_pos==0)
        return false; // no actual update takes place

    for(int i=0; i<3; i++)
    {
        FixedPointValue fpv(src.m_entity_data.pos[i]);
        //diff = packed ^ prev_pos[i]; // changed bits are '1'
        bs.StoreBits(24,fpv.store);
    }
    return true;
}
bool update_rot(const Entity &src, int axis ) /* returns true if given axis needs updating */
{
    if(axis==axis)
        return true;
    return false;
}
void storeOrientation(const Entity &src,BitStream &bs)
{
    // Check if update needed through update_rot()
    uint8_t updates;
    updates = ((uint8_t)update_rot(src,0)) | (((uint8_t)update_rot(src,1))<<1) | (((uint8_t)update_rot(src,2))<<2);
    storeBitsConditional(bs,3,updates); //frank 7,0,0.1,0
#ifdef DEBUG_INPUT
    fprintf(stderr,"\nupdates: %i\n",updates);
#endif
    float pyr_angles[3];
    glm::vec3 vec = toCoH_YPR(src.m_direction);
    pyr_angles[0] = 0.0f;
    pyr_angles[1] = vec.y; // set only yaw value
    pyr_angles[2] = 0.0f;
#ifdef DEBUG_ORIENTATION
    // output everything
    fprintf(stderr,"\nPlayer: %d\n",src.m_idx);
    fprintf(stderr,"dir: %s \n", glm::to_string(src.direction).c_str());
    fprintf(stderr,"camera_pyr: %s \n", glm::to_string(src.inp_state.camera_pyr).c_str());
    fprintf(stderr,"pyr_angles: farr(%f, %f, %f)\n", pyr_angles[0], pyr_angles[1], pyr_angles[2]);
    fprintf(stderr,"orient_p: %f \n", src.m_entity_data.m_orientation_pyr[0]);
    fprintf(stderr,"orient_y: %f \n", src.m_entity_data.m_orientation_pyr[1]);
    fprintf(stderr,"vel_scale: %f \n", src.inp_state.input_vel_scale);
#endif
    for(int i=0; i<3; i++)
    {
        if(update_rot(src,i))
        {
            uint32_t v;
            v = AngleQuantize(pyr_angles[i],9);
#ifdef DEBUG_INPUT
            fprintf(stderr,"v: %d\n", v); // does `v` fall between 0...512
#endif
            bs.StoreBits(9,v);
        }
    }
}

void storePosUpdate(const Entity &src, bool just_created, BitStream &bs)
{
    bool extra_info = false;
    bool move_instantly = false;
    PUTDEBUG("before entReceivePosUpdate");

    bool position_updated = storePosition(src,bs);
    PUTDEBUG("before posInterpolators");
    if(!just_created && position_updated)
    {
        // if position has changed
        // prepare interpolation table, given previous position
        bs.StoreBits(1,extra_info); // not extra_info
        if(extra_info) {
            bs.StoreBits(1,move_instantly);
            // Bintree sending happens here
            storeUnknownBinTree(src,bs);
        }
        // if extra_inf
    }
    PUTDEBUG("before storeOrientation");
    storeOrientation(src,bs);
    PUTDEBUG("after storeOrientation");

}
void sendSeqMoveUpdate(const Entity &src,BitStream &bs)
{
    //ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\tSending seq mode update %d\n"),m_seq_update));
    PUTDEBUG("before sendSeqMoveUpdate");

    bs.StoreBits(1,src.m_seq_update); // no seq update
    if(src.m_seq_update)
    {
        storePackedBitsConditional(bs,8,src.m_seq_upd_num1); // move index
        storePackedBitsConditional(bs,4,src.m_seq_upd_num2); //maxval is 255
    }
}
void sendSeqTriggeredMoves(const Entity &src,BitStream &bs)
{
    PUTDEBUG("before sendSeqTriggeredMoves");
    uint32_t num_moves=0;
    //ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\tSending seq triggeted moves %d\n"),num_moves));
    bs.StorePackedBits(1,num_moves); // num moves
    for (uint32_t idx = 0; idx < num_moves; ++idx )
    {
        bs.StorePackedBits(16,0); // 2  EntityStoredMoveP->field_2
        bs.StorePackedBits(6,0); //0  EntityStoredMoveP->field_0
        storePackedBitsConditional(bs,16,0); // 1 EntityStoredMoveP->field_1
    }
}
void sendNetFx(const Entity &src,BitStream &bs)
{
    bs.StorePackedBits(1,src.m_num_fx); // num fx
    //NetFx.serializeto();
    for(int i=0; i<src.m_num_fx; i++)
    {
        bs.StoreBits(8,src.m_fx1[i]); // command
        bs.StoreBits(32,src.m_fx2[i]); // NetID
        bs.StoreBits(1,0);
        storePackedBitsConditional(bs,10,0xCB8); // handle
        storeBitsConditional(bs,4,0); // client timer
        storeBitsConditional(bs,32,0); // clientTriggerFx
        storeFloatConditional(bs,0.0); // duration
        storeFloatConditional(bs,10.0); // radius
        storeBitsConditional(bs,4,10);  // power
        storeBitsConditional(bs,32,0);  // debris
        int val=0;
        storeBitsConditional(bs,2,val); // origiType
        if(val==1)
        {
            bs.StoreFloat(0.0); // origin Pos
            bs.StoreFloat(0.0);
            bs.StoreFloat(0.0);
        }
        else
        {
            if(val)
            {
                //"netbug"
            }
            else
            {
                storePackedBitsConditional(bs,8,0); // origin entity
                bs.StorePackedBits(2,0); // bone id

            }
        }
        storeBitsConditional(bs,2,0); // target type
        if(false)
        {
            bs.StoreFloat(0); // targetPos
            bs.StoreFloat(0);
            bs.StoreFloat(0);
        }
        else
        {
            storePackedBitsConditional(bs,12,0x19b); // target entity
        }
    }
}
void sendCostumes(const Entity &src,BitStream &bs)
{
    //NOTE: this will only be initialized once, and no changes later on will influence this
    static ColorAndPartPacker *packer = g_GlobalMapServer->runtimeData().getPacker();
    PUTDEBUG("before sendCostumes");
    storePackedBitsConditional(bs,2,src.m_costume_type);
    if(src.m_costume_type!=1)
    {
        assert(false);
        return;
    }
    switch(src.m_type)
    {
        case Entity::ENT_PLAYER: // client value 1
            src.m_char->serialize_costumes(bs,packer,true); // we're always sending full info
            break;
        case 3: // client value 2 top level defs from VillainCostume ?
            bs.StorePackedBits(12,1); // npc costume type idx ?
            bs.StorePackedBits(1,1); // npc costume idx ?
            break;
        case Entity::ENT_CRITTER: // client val 4
            bs.StoreString("Unknown"); // TODO what is stored here?
            break;
    }
}
void sendXLuency(BitStream &bs,float val)
{
    storeBitsConditional(bs,8,std::min(static_cast<int>(uint8_t(val*255)),255));
}
void sendCharacterStats(const Entity &src,BitStream &bs)
{
    bool have_stats=true; // no stats -> dead ?
    bool stats_changed=true;
    bool we_have_a_buddy = false;
    bool our_buddy_is_our_mentor = false;
    bool we_have_our_buddy_dbid=false;
    int our_buddy_dbid = 0;
    bs.StoreBits(1,have_stats); // nothing here for now
    if(!have_stats)
        return;
    bs.StoreBits(1,stats_changed);
    if(!stats_changed)
        return;
    bs.StoreBits(1,we_have_a_buddy);
    if ( we_have_a_buddy )        // buddy info
    {
        bs.StoreBits(1,our_buddy_is_our_mentor);
        bs.StoreBits(1,we_have_our_buddy_dbid);
        if(we_have_our_buddy_dbid)
        {
           bs.StorePackedBits(20,our_buddy_dbid);
        }
    }
    serializeStats(*src.m_char,bs,false);
}
void sendBuffsConditional(const Entity &src,BitStream &bs)
{
    //TODO: implement this
    bs.StoreBits(1,0); // nothing here for now
    if(false)
    {
        sendBuffs(src,bs);
    }
}
void sendTargetUpdate(const Entity &src,BitStream &bs)
{
    uint32_t assist_id  = getAssistTargetIdx(src);
    uint32_t target_id  = getTargetIdx(src);
    bool has_target     = src.m_has_target;

    bs.StoreBits(1,has_target); // TODO: test this
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
    bs.StoreBits(1,src.m_odd_send);
}
void sendWhichSideOfTheForce(const Entity &src,BitStream &bs)
{
    bs.StoreBits(1,0); // on team evil ?
    bs.StoreBits(1,1); // on team good ?
}
void sendEntCollision(const Entity &src,BitStream &bs)
{
    // if 1 is sent, client will disregard it's own collision processing.
    bs.StoreBits(1,0); // 1/0 only
}
void sendNoDrawOnClient(const Entity &src,BitStream &bs)
{
    bs.StoreBits(1,0); // 1/0 only
}
void sendAFK(const Entity &src, BitStream &bs)
{
    CharacterData cd = src.m_char->m_char_data;
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
    bs.StoreBits(1,src.m_supergroup.m_SG_info); // UNFINISHED
    if(!src.m_supergroup.m_SG_info)
        return;
    bs.StorePackedBits(2,src.m_supergroup.m_SG_id);
    if(src.m_supergroup.m_SG_id)
    {
        bs.StoreString(src.m_supergroup.m_SG_name);//64 chars max
        bs.StoreString("");//128 chars max -> hash table key from the CostumeString_HTable
        bs.StoreBits(32,src.m_supergroup.m_SG_color1); // supergroup color 1
        bs.StoreBits(32,src.m_supergroup.m_SG_color2); // supergroup color 2
    }
}
void sendLogoutUpdate(const Entity &src,ClientEntityStateBelief &belief,BitStream &bs)
{
    if(belief.m_is_logging_out==src.m_is_logging_out) // no change in logout state
    {
        bs.StoreBits(1,false);
        return;
    }
    bs.StoreBits(1,true); // logout state update
    bs.StoreBits(1,0); // if 1 then it means the logout was caused by low connection quality.
    // we send 0 as a time to logout if this is a logout-abort
    storePackedBitsConditional(bs,5,src.m_is_logging_out ? src.m_time_till_logout/(1000) : 0);
    belief.m_is_logging_out = src.m_is_logging_out;
}

////////////////////////////////////////////////////////////////////////////////////////////
} // end of anonoymous namespace
void sendBuffs(const Entity &src,BitStream &bs)
{
    bs.StorePackedBits(5,0);
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

    bs.StoreBits(1,src.might_have_rare); //var_C

    if(src.might_have_rare)
        bs.StoreBits(1,src.m_rare_bits);

    if(src.m_rare_bits)
        sendStateMode(src,bs);

    storePosUpdate(src,update_existence && ent_exists, bs);

    if(src.might_have_rare)
        sendSeqMoveUpdate(src,bs);

    if(src.m_rare_bits)
        sendSeqTriggeredMoves(src,bs);

    // NPC -> m_pchar_things=0 ?
    PUTDEBUG("before m_pchar_things");
    bs.StoreBits(1,src.m_pchar_things);
    if(src.m_pchar_things)
    {
        sendNetFx(src,bs);
    }
    if(src.m_rare_bits)
    {
        sendCostumes(src,bs);
        sendXLuency(bs,src.translucency);
        src.m_char->sendTitles(bs,NameFlag::HasName,ConditionalFlag::Conditional);
    }
    if(src.m_pchar_things)
    {
        sendCharacterStats(src,bs);
        sendBuffsConditional(src,bs);
        sendTargetUpdate(src,bs);
    }
    if(src.m_rare_bits)
    {
        sendOnOddSend(src,bs); // is one on client end
        sendWhichSideOfTheForce(src,bs);
        sendEntCollision(src,bs);
        sendNoDrawOnClient(src,bs);
        sendAFK(src,bs);
        sendOtherSupergroupInfo(src,bs);
        sendLogoutUpdate(src,belief,bs);
    }
}
