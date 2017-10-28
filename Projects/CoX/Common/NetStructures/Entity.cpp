/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#define _USE_MATH_DEFINES
#include "Entity.h"

#include <ace/Log_Msg.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
//#define LOG_
//TODO: this file needs to know the MapInstance's WorldSimulation rate - Maybe extract it as a configuration object ?

#define WORLD_UPDATE_TICKS_PER_SECOND 30
static const float F_PI = float(M_PI); // to prevent double <-> float conversion warnings

float AngleDequantize(int value,int numb_bits)
{
    int max_val = 1<<numb_bits;
    float v = F_PI*(float(value)/max_val) - F_PI/2;
    if(v<(-F_PI/2))
        return -1.0f;
    else if(v>(F_PI/2))
        return 1.0f;
    else if(v<0.00001f)
        return 0.0f;
    return sinf(v);
}
uint32_t AngleQuantize(float value,int numb_bits)
{
    int max_val = 1<<numb_bits;

    float v = fabs(value)>1.0f ? std::copysign(1.0f,value) : value ;
    v  = (asinf(v)+M_PI)/(2*F_PI); // maps -1..1 to 0..1
    v *= max_val;
//  assert(v<=max_val);
    return uint32_t(v);
}
int Entity::getOrientation(BitStream &bs)
{
    float fval;
    int update_qrot;
    update_qrot = getBitsConditional(bs,3);
    if(!update_qrot)
        return 0;
    bool recv_older = false;
    for(int i=0; i<3; i++)
    {
        if(update_qrot&(1<<i))
        {
            fval = AngleDequantize(bs.GetBits(9),9);
            if(current_client_packet_id>pkt_id_QrotUpdateVal[i])
            {
                pkt_id_QrotUpdateVal[i] = current_client_packet_id;
                qrot[i] = fval;
            }
            else
                recv_older=true;
        }
    }
    //RestoreFourthQuatComponent(pEnt->qrot);
    //NormalizeQuaternion(pEnt->qrot)
    return recv_older==false;
}
static void toEulerAngle(const glm::quat& q, float& roll, float& pitch, float& yaw)
{
    // roll (x-axis rotation)
    float sinr = 2.0f * (q.w * q.x + q.y * q.z);
    float cosr = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    roll = std::atan2(sinr, cosr);

    // pitch (y-axis rotation)
    float sinp = 2.0f * (q.w * q.y - q.z * q.x);
        if (std::abs(sinp) >= 1)
            pitch = std::copysign(float(M_PI / 2), sinp); // use 90 degrees if out of range
        else
        pitch = std::asin(sinp);

    // yaw (z-axis rotation)
    float siny = 2.0f * (q.w * q.z + q.x * q.y);
    float cosy = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    yaw = std::atan2(siny, cosy);
}
void Entity::storeOrientation(BitStream &bs) const
{
    // if(updateNeeded())
    uint8_t updates;
    updates = ((uint8_t)update_rot(0)) | (((uint8_t)update_rot(1))<<1) | (((uint8_t)update_rot(2))<<2);
    storeBitsConditional(bs,3,updates); //frank 7,0,0.1,0
    //NormalizeQuaternion(pEnt->qrot)
    //
    //RestoreFourthQuatComponent(pEnt->qrot);
    float pyr_angles[3];
    toEulerAngle(qrot,pyr_angles[0],pyr_angles[1],pyr_angles[2]);
    for(int i=0; i<3; i++)
    {
        if(update_rot(i))
        {
            bs.StoreBits(9,AngleQuantize(pyr_angles[i],9));   // normalized quat, 4th param is recoverable from the first 3
        }
    }
}
static inline uint32_t quantize_float(float v)
{
    return uint32_t(floorf(v*64)+0x800000);
}
void Entity::storePosition(BitStream &bs) const
{
// float x = pos.vals.x;
    uint32_t packed;
    //uint32_t diff=0; // changed bits are '1'
    bs.StoreBits(3,7);
    for(int i=0; i<3; i++)
    {
        packed = quantize_float(pos[i]);
        packed = packed<0xFFFFFF ? packed : 0xFFFFFF;
        //diff = packed ^ prev_pos[i]; // changed bits are '1'
        bs.StoreBits(24,packed);
    }
}
struct BinTreeEntry {
    uint8_t x,y,z,d;
};
struct BinTreeBase {
    BinTreeEntry arr[7];
};
void Entity::storePosUpdate(BitStream &bs) const
{
    bool extra_info = false;
    bool move_instantly = false;
    storePosition(bs);
    if(!m_change_existence_state)
    {
        // if position has changed
        // prepare interpolation table, given previous position
        bs.StoreBits(1,extra_info); // not extra_info
        if(extra_info) {
            bs.StoreBits(1,move_instantly);
            // Bintree sending happens here
            storeUnknownBinTree(bs);
        }
        // if extra_inf
    }
    storeOrientation(bs);
}
void Entity::storeUnknownBinTree(BitStream &bs) const
{
    bs.StoreBits(1,0);

}
void Entity::sendStateMode(BitStream &bs) const
{
    PUTDEBUG("before sendStateMode");
    bs.StoreBits(1,m_state_mode_send); // no state mode
    PUTDEBUG("before sendStateMode 2");
    if(m_state_mode_send)
    {
        storePackedBitsConditional(bs,3,m_state_mode);
    }
    PUTDEBUG("after sendStateMode");
}
void Entity::sendSeqMoveUpdate(BitStream &bs) const
{
    //ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\tSending seq mode update %d\n"),m_seq_update));
    bs.StoreBits(1,m_seq_update); // no seq update
    if(m_seq_update)
    {
        storePackedBitsConditional(bs,8,m_seq_upd_num1); // move index
        storePackedBitsConditional(bs,4,m_seq_upd_num2); //maxval is 255

    }
}
void Entity::sendSeqTriggeredMoves(BitStream &bs) const
{
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
void Entity::sendNetFx(BitStream &bs) const
{
    bs.StorePackedBits(1,m_num_fx); // num fx
    //NetFx.serializeto();
    for(int i=0; i<m_num_fx; i++)
    {
        bs.StoreBits(8,m_fx1[i]); // command
        bs.StoreBits(32,m_fx2[i]); // NetID
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
void Entity::sendCostumes(BitStream &bs,ColorAndPartPacker *packer) const
{
    storePackedBitsConditional(bs,2,m_costume_type);
    if(m_costume_type!=1)
    {
        assert(false);
        return;
    }
    switch(m_type)
    {
        case ENT_PLAYER: // client value 1
            m_char.serialize_costumes(bs,packer,true); // we're always sending full info
            break;
        case 3: // client value 2 top level defs from VillainCostume ?
            bs.StorePackedBits(12,1); // npc costume type idx ?
            bs.StorePackedBits(1,1); // npc costume idx ?
            break;
        case ENT_CRITTER: // client val 4
            bs.StoreString("Unknown");
            break;
    }
}

PlayerEntity::PlayerEntity()
{
    m_costume_type   = 1;
    m_change_existence_state         = true;
    m_destroyed         = false;
    m_type           = 2; // PLAYER
    m_create_player  = true;
    m_player_villain = false;
    m_origin_idx     = 0;
    m_class_idx      = 0;
    m_selector1      = false;
    m_hasname        = true;
    m_hasgroup_name  = false;
    m_pchar_things   = false;

    m_char.reset();
    might_have_rare = m_rare_bits = true;
}

void Entity::sendXLuency(BitStream &bs,float val) const
{
    storeBitsConditional(bs,8,std::min(static_cast<int>(uint8_t(val*255)),255));
}
void Entity::sendTitles(BitStream &bs) const
{
    bs.StoreBits(1,m_has_titles); // no titles
    if(!m_has_titles)
        return;
    if(m_type==ENT_PLAYER)
    {
        bs.StoreString(m_char.getName());
        bs.StoreBits(1,0); // ent_player2->flag_F4
        storeStringConditional(bs,"");//max 32 chars // warcry ?
        storeStringConditional(bs,"");//max 32 chars
        storeStringConditional(bs,"");//max 128 chars
    }
    else // unused
    {
        bs.StoreString("");
        bs.StoreBits(1,0);
        storeStringConditional(bs,"");
        storeStringConditional(bs,"");
        storeStringConditional(bs,"");
    }
}
void Entity::sendRagDoll(BitStream &bs) const
{
    int num_bones=0; //NPC->0 bones
    storeBitsConditional(bs,5,num_bones);
    if(num_bones)
        bs.StorePackedBits(1,0); // no titles
}
void Entity::sendOnOddSend(BitStream &bs,bool is_odd) const
{
    bs.StoreBits(1,is_odd);
}
void Entity::sendAllyID(BitStream &bs)
{
    bs.StorePackedBits(2,0);
    bs.StorePackedBits(4,0); // NPC->0

}
void Entity::sendPvP(BitStream &bs)
{
    bs.StoreBits(1,0);
    bs.StoreBits(1,0);
    bs.StorePackedBits(5,0);
    bs.StoreBits(1,0);
}
void Entity::sendEntCollision(BitStream &bs) const
{
    bs.StoreBits(1,0); // 1/0 only
}
void Entity::sendNoDrawOnClient(BitStream &bs) const
{
    bs.StoreBits(1,0); // 1/0 only
}
void Entity::sendContactOrPnpc(BitStream &bs) const
{
    // frank 1
    bs.StoreBits(1,m_contact); // 1/0 only
}
void Entity::sendPetName(BitStream &bs) const
{
    storeStringConditional(bs,"");
}
void Entity::sendAFK(BitStream &bs) const
{
    bool is_away=false;
    bool away_string=false;
    bs.StoreBits(1,is_away); // 1/0 only
    if(is_away)
    {
        bs.StoreBits(1,away_string); // 1/0 only
        if(away_string)
            bs.StoreString("");
    }
}
void Entity::sendOtherSupergroupInfo(BitStream &bs) const
{
    bs.StoreBits(1,m_SG_info); // UNFINISHED
    if(m_SG_info) // frank has info
    {
        bs.StorePackedBits(2,field_78);
        if(field_78)
        {
            bs.StoreString("");//64 chars max
            bs.StoreString("");//128 chars max -> hash table key from the CostumeString_HTable
            bs.StoreBits(32,0);
            bs.StoreBits(32,0);
        }
    }
}
void Entity::sendLogoutUpdate(BitStream &bs) const
{
    if(m_logout_sent) {
        bs.StoreBits(1,false);
        return;
    }
    bs.StoreBits(1,m_is_logging_out);
    if(m_is_logging_out)
    {
        m_logout_sent = true;
        bs.StoreBits(1,0); // if 1 then it means the logout was caused by low connection quality.
        storePackedBitsConditional(bs,5,m_time_till_logout/(1000)); // time to logout, multiplied by 30
        printf("LOGOUT TIME %d\n",m_time_till_logout);
    }
}

void Entity::storeCreation( BitStream &bs) const
{
    // entity creation
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\tSending create entity\n")));

    bs.StoreBits(1,m_destroyed); // ends creation destroys seq and returns NULL

    if(m_destroyed)
        return;

    bs.StorePackedBits(12,m_idx);//  this will be put in  of created entity
    PUTDEBUG("after id");
    bs.StorePackedBits(2,m_type);
    if(m_type==ENT_PLAYER)
    {
        bs.StoreBits(1,m_create_player);
        if(m_create_player)
            bs.StorePackedBits(1,m_access_level);
        bs.StorePackedBits(20,m_idx);//TODO: should be bs.StorePackedBits(20,m_db_id);
    }
    else
    {
        bool val=false;
        bs.StoreBits(1,val);
        if(val)
        {
            bs.StorePackedBits(12,ownerEntityId); // entity idx
            bs.StorePackedBits(12,creatorEntityId); // entity idx
        }
    }
    PUTDEBUG("after creatorowner");
    if(m_type==ENT_PLAYER || m_type==ENT_CRITTER)
    {
        bs.StorePackedBits(1,m_origin_idx);
        bs.StorePackedBits(1,m_class_idx);
        bool hasTitle=false;
        bs.StoreBits(1,hasTitle);
        if(hasTitle)
        {
            bs.StoreBits(1,0); // likely an index to a title prefix ( 1 - The )
            storeStringConditional(bs,""); //title1
            storeStringConditional(bs,""); //title2
            storeStringConditional(bs,""); //title3
        }
    }
    bs.StoreBits(1,m_hasname);
    if(m_hasname)
        bs.StoreString(m_char.getName());
    PUTDEBUG("after names");
    bool fadin = true;
    bs.StoreBits(1,fadin); // Is entity being faded in ?
    // the following is used as an input to LCG float generator, generated float (0-1) is used as
    // linear interpolation factor betwwen scale_min and scale_max
    bs.StoreBits(32,m_randSeed);
    bs.StoreBits(1,m_hasgroup_name);
    if(m_hasgroup_name)
    {
        int rank=0; // rank in the group ?
        bs.StorePackedBits(2,rank);// this will be put in field_1830 of created entity
        bs.StoreString(m_group_name);
    }
    PUTDEBUG("end storeCreation");
}
void serializeto(const Entity & src, BitStream &bs,ColorAndPartPacker *packer )
{
    //////////////////////////////////////////////////////////////////////////
    bs.StoreBits(1,src.m_change_existence_state);
    if(src.m_change_existence_state)
        src.storeCreation(bs);
    //////////////////////////////////////////////////////////////////////////
    // creation ends here
    PUTDEBUG("before entReceiveStateMode");

    bs.StoreBits(1,src.might_have_rare); //var_C

    if(src.might_have_rare)
        bs.StoreBits(1,src.m_rare_bits);

    if(src.m_rare_bits)
        src.sendStateMode(bs);

    PUTDEBUG("before entReceivePosUpdate");
    src.storePosUpdate(bs);

    if(src.might_have_rare)
        src.sendSeqMoveUpdate(bs);

    if(src.m_rare_bits)
        src.sendSeqTriggeredMoves(bs);

    // NPC -> m_pchar_things=0 ?
    bs.StoreBits(1,src.m_pchar_things);
    if(src.m_pchar_things)
    {
        src.sendNetFx(bs);
    }
    if(src.m_rare_bits)
    {
        src.sendCostumes(bs,packer);
        src.sendXLuency(bs,src.translucency);
        src.sendTitles(bs);
    }
    if(src.m_pchar_things)
    {
        src.sendCharacterStats(bs);
        src.sendBuffsConditional(bs);
        src.sendTargetUpdate(bs);
    }
    if(src.m_rare_bits)
    {
        src.sendOnOddSend(bs,src.m_odd_send); // is one on client end
        src.sendWhichSideOfTheForce(bs);
        src.sendEntCollision(bs);
        src.sendNoDrawOnClient(bs);
        src.sendAFK(bs);
        src.sendOtherSupergroupInfo(bs);
        src.sendLogoutUpdate(bs);
    }
}
void Entity::sendBuffs(BitStream &bs) const
{
    bs.StorePackedBits(5,0);
}

void Entity::fillFromCharacter(Character *f)
{
    m_char = *f;
    m_hasname = true;
    //TODO: map class/origin name to Entity's class/orign indices.
}
/**
 *  This will mark the Entity as being in logging out state
 *  \arg time_till_logout is time in seconds untill logout is done
 */
void Entity::beginLogout(uint16_t time_till_logout)
{
    m_is_logging_out = true;
    m_time_till_logout = time_till_logout*1000;
}
void Entity::sendBuffsConditional(BitStream &bs) const
{
    bs.StoreBits(1,0); // nothing here for now
    if(false)
    {
        sendBuffs(bs);
    }
}
void Entity::sendCharacterStats(BitStream &bs) const
{
    bs.StoreBits(1,0); // nothing here for now
}
void Entity::sendTargetUpdate(BitStream &bs) const
{
    bs.StoreBits(1,0); // nothing here for now
}
void Entity::sendWhichSideOfTheForce(BitStream &bs) const
{
    bs.StoreBits(1,0);
    bs.StoreBits(1,0);
    // flags 3 & 2 of entity flags_1
}
bool Entity::update_rot( int axis ) const /* returns true if given axis needs updating */
{
    if(axis==axis)
        return true;
    return false;
}

void PlayerEntity::serializefrom_newchar( BitStream &src,ColorAndPartPacker *packer )
{
    /*int val =*/ src.GetPackedBits(1); //2
    m_char.GetCharBuildInfo(src);
    m_char.recv_initial_costume(src,packer);
    /*int t =*/ src.GetBits(1); // The -> 1
    src.GetString(m_battle_cry);
    src.GetString(m_character_description);
}
void PlayerEntity::serialize_full( BitStream &bs )
{
    m_char.SendCharBuildInfo(bs); //FIXEDOFFSET_pchar->character_Receive
    PUTDEBUG("PlayerEntity::serialize_full before sendFullStats");
    m_char.sendFullStats(bs); //Entity::receiveFullStats(&FullStatsTokens, pak, FIXEDOFFSET_pchar, pkt_id_fullAttrDef, 1);
    PUTDEBUG("PlayerEntity::serialize_full before sendBuffs");
    sendBuffs(bs); //FIXEDOFFSET_pchar->character_ReceiveBuffs(pak,0);
    PUTDEBUG("PlayerEntity::serialize_full before sidekick");
    bool has_sidekick=false;
    bs.StoreBits(1,has_sidekick);
    if(has_sidekick)
    {
        bool is_mentor=false; // this flag might mean something totally different :)
        bs.StoreBits(1,is_mentor);
        bs.StorePackedBits(20,0); // sidekick partner db_id -> 10240
    }
}
void Entity::InsertUpdate( PosUpdate pup )
{
    m_update_idx++;
    m_update_idx %=64;
    m_pos_updates[m_update_idx]=pup;
}

void Entity::dump()
{
    m_char.dump();
}

void Entity::addPosUpdate(const PosUpdate & p) {
    m_update_idx = (m_update_idx+1) % 64;
    m_pos_updates[m_update_idx] = p;
}

void Entity::addInterp(const PosUpdate & p) {
    interpResults.emplace_back(p);
}

Entity::Entity()
{
    m_access_level = 9; // enables access to all deve
    pos = glm::vec3(-60.5f,180.0f,0.0f);
}

void sendPower(BitStream &bs,int a,int b,int c)
{
    bs.StorePackedBits(3,a);
    bs.StorePackedBits(3,b);
    bs.StorePackedBits(3,c);
}
void sendPowers(BitStream &bs)
{
    bs.StorePackedBits(4,0); // count
    for(int i=0; i<0; i++)
    {
        uint32_t num_powers=0;
        bs.StorePackedBits(5,0);
        bs.StorePackedBits(4,uint32_t(num_powers));
        for(uint32_t idx=0; idx<num_powers; ++idx)
        {
            uint32_t num_somethings=0;
            sendPower(bs,0,0,0);

            bs.StorePackedBits(5,0);
            bs.StoreFloat(1.0);
            bs.StorePackedBits(4,num_somethings);

            for(size_t idx2=0; idx2<num_somethings; ++idx2)
            {
                sendPower(bs,0,0,0);
                bs.StorePackedBits(5,0);
                bs.StorePackedBits(2,0);
            }
        }
    }
}
void sendPowers_main_tray(BitStream &bs)
{
    uint32_t max_num_cols=3;
    uint32_t max_num_rows=1;
    bs.StorePackedBits(3,max_num_cols); // count
    bs.StorePackedBits(3,max_num_rows); // count
    for(uint32_t i=0; i<max_num_cols; i++)
    {
        for(size_t idx=0; idx<max_num_rows; ++idx)
        {
            bool is_power=false;
            bs.StoreBits(1,is_power);
            if(is_power)
            {
                sendPower(bs,0,0,0);
            }
        }
    }
}

static void sendBoosts(BitStream &bs)
{
    uint32_t num_boosts=0;
    bs.StorePackedBits(5,num_boosts); // count
    for(size_t idx=0; idx<num_boosts; ++idx)
    {
        bool set_boost=false;
        bs.StorePackedBits(3,0); // bost idx
        bs.uStoreBits(1,set_boost); // 1 set, 0 clear
        if(set_boost)
        {
            sendPower(bs,0,0,0);
            bs.StorePackedBits(5,0); // bost idx
            bs.StorePackedBits(2,0); // bost idx
        }
    }
    // boosts
}
void sendUnk2(BitStream &bs)
{
    bs.StorePackedBits(5,0); // count
}
void sendUnk3(BitStream &bs) // inventory ?
{
    bs.StorePackedBits(3,0); // count
}

void abortLogout(Entity *e)
{
    e->m_logout_sent = false;
    e->m_is_logging_out = true; // send logout time of 0
    e->m_time_till_logout = 0;
}
