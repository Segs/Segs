/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#define _USE_MATH_DEFINES
#include "Entity.h"

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
void Entity::storeOrientation(BitStream &bs) const
{
    // if(updateNeeded())
    uint8_t updates;
    updates = ((uint8_t)update_rot(0)) | (((uint8_t)update_rot(1))<<1) | (((uint8_t)update_rot(2))<<2);
    storeBitsConditional(bs,3,updates); //frank 7,0,0.1,0
    //NormalizeQuaternion(pEnt->qrot)
    //
    //RestoreFourthQuatComponent(pEnt->qrot);
    for(int i=0; i<3; i++)
    {
        if(update_rot(i))
        {
            bs.StoreBits(9,AngleQuantize(qrot[i],9));   // normalized quat, 4th param is recoverable from the first 3
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
    bs.StoreBits(3,7); // frank -> 7,-60.5,0,180
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
    // if(is_update)

    if(!m_create)
    {
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
    //ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\tSending state mode\n")));

    bs.StoreBits(1,m_state_mode_send); // no state mode
    if(m_state_mode_send)
    {
        storePackedBitsConditional(bs,3,m_state_mode);
    }
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
void MobEntity::sendCostumes( BitStream &bs ) const
{
    int npc_costume_type_idx=0;
    int costume_idx=0;
    storePackedBitsConditional(bs,2,m_costume_type);
    if((m_costume_type!=2)&&(m_costume_type!=4))
    {
        assert(false);
        return;
    }
    if(m_costume_type==2)
    {
        bs.StorePackedBits(12,npc_costume_type_idx);
        bs.StorePackedBits(1,costume_idx);
    }
    else
    {
        bs.StoreString(m_costume_seq);
    }
}

void Entity::sendCostumes(BitStream &bs) const
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
        m_char.serialize_costumes(bs,true); // we're always sending full info
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
    m_costume_type=1;
    m_create=true;
    var_129C=false;
    m_type = 2; //PLAYER
    m_create_player=true;
    m_player_villain=false;
    m_origin_idx=m_class_idx=0;
    m_selector1=false;
    m_hasname = true;
    m_hasgroup_name=false;
    m_pchar_things=false;

    m_char.reset();
    might_have_rare=m_rare_bits = true;
}
void PlayerEntity::sendCostumes( BitStream &bs ) const
{
    Entity::sendCostumes(bs);
}
MobEntity::MobEntity()
{
    m_costume_type=2;
}
void Entity::sendXLuency(BitStream &bs,float val) const
{
    storeBitsConditional(bs,8,std::min<>(static_cast<int>(uint8_t(val*255)),255)); // upto here everything is ok
}
void Entity::sendTitles(BitStream &bs) const
{
    bs.StoreBits(1,m_has_titles); // no titles
    if(m_has_titles)
    {
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
        bs.StoreBits(1,0); // flags_1[1] set in entity
        storePackedBitsConditional(bs,5,m_time_till_logout/(1000)); // time to logout, multiplied by 30
        printf("LOGOUT TIME %d\n",m_time_till_logout);
    }
}

void Entity::storeCreation( BitStream &bs) const
{

    // entity creation
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\tSending create entity\n")));

        bs.StoreBits(1,var_129C); // checkEntCreate_var_129C / ends creation destroys seq and returns NULL

        if(var_129C)
            return;

        //bs.StorePackedBits(12,field_64);//  this will be put in  of created entity
        bs.StorePackedBits(12,m_idx);//  this will be put in  of created entity
        bs.StorePackedBits(2,m_type);
        if(m_type==ENT_PLAYER)
        {
            bs.StoreBits(1,m_create_player);
            if(m_create_player)
                bs.StorePackedBits(1,m_access_level);
            bs.StorePackedBits(20,m_idx);//bs.StorePackedBits(20,m_db_id); //intptr_t(this)&0xFFFFF
        }
        else
        {
            bool val=false;
            bs.StoreBits(1,val);
            if(val)
            {
                bs.StorePackedBits(12,0); // entity idx
                bs.StorePackedBits(12,0); // entity idx
            }
        }
        if(m_type==ENT_PLAYER || m_type==ENT_CRITTER)
        {
            bs.StorePackedBits(1,m_origin_idx);
            bs.StorePackedBits(1,m_class_idx);
            bool val=false;
            bs.StoreBits(1,val);
            if(val)
            {
                bs.StoreBits(1,0); // entplayer_flgFE0
                storeStringConditional(bs,""); //title1
                storeStringConditional(bs,""); //title2
                storeStringConditional(bs,""); //title3
            }
        }
        bs.StoreBits(1,m_hasname);
        if(m_hasname)
            bs.StoreString(m_char.getName());
        bs.StoreBits(1,1); //var_94 if set Entity.field_1818/field_1840=0 else field_1818/field_1840 = 255,2
        // the following is used as an input to LCG float generator, generated float (0-1) is used as
        // linear interpolation factor betwwen scale_min and scale_max
        bs.StoreBits(32,field_60); // this will be put in field_60 of created entity
        bs.StoreBits(1,m_hasgroup_name);
        if(m_hasgroup_name)
        {
            bs.StorePackedBits(2,0);// this will be put in field_1830 of created entity
            bs.StoreString(m_group_name);
        }
    }
void Entity::serializeto( BitStream &bs ) const
{
#ifdef LOG_
    printf("Serializing entity with idx %d\n",m_idx);
#endif
    //////////////////////////////////////////////////////////////////////////
    bs.StoreBits(1,m_create);
    if(m_create)
        storeCreation(bs);
    //////////////////////////////////////////////////////////////////////////
    // creation ends here

    bs.StoreBits(1,might_have_rare); //var_C

    if(might_have_rare)
        bs.StoreBits(1,m_rare_bits);

    if(m_rare_bits)
        sendStateMode(bs);

    storePosUpdate(bs);

    if(might_have_rare)
        sendSeqMoveUpdate(bs);

    if(m_rare_bits)
        sendSeqTriggeredMoves(bs);

    // NPC -> m_pchar_things=0 ?
    bs.StoreBits(1,m_pchar_things);
    if(m_pchar_things)
    {
        sendNetFx(bs);
    }
    if(m_rare_bits)
    {
        sendCostumes(bs);
        sendXLuency(bs,0.5f);
        sendTitles(bs);
    }
    if(m_pchar_things)
    {
        sendCharacterStats(bs);
        sendBuffsConditional(bs);
        sendTargetUpdate(bs);
    }
    if(m_rare_bits)
    {
        sendOnOddSend(bs,m_odd_send); // is one on client end
        sendWhichSideOfTheForce(bs);
        sendEntCollision(bs);
        sendNoDrawOnClient(bs);
        sendAFK(bs);
        sendOtherSupergroupInfo(bs);
        sendLogoutUpdate(bs);
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
    if(0)
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
/*
void Avatar::DumpPowerPoolInfo( const PowerPool_Info &pool_info )
{
    for(int i=0; i<3; i++)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Pool_id[%d]: 0x%08x\n"),i,pool_info.id[i]));
    }
}
*/

/*
void Avatar::DumpBuildInfo()
{
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    class: %s\n"),m_class_name.c_str()));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    origin: %s\n"),m_origin_name.c_str()));
    DumpPowerPoolInfo(m_powers[0]);
    DumpPowerPoolInfo(m_powers[1]);
}
*/

void PlayerEntity::serializefrom_newchar( BitStream &src )
{
    /*int val =*/ src.GetPackedBits(1); //2
    m_char.GetCharBuildInfo(src);
    m_char.recv_initial_costume(src);
    /*int t =*/ src.GetBits(1); // The -> 1
    src.GetString(m_battle_cry);
    src.GetString(m_character_description);
}
void PlayerEntity::serialize_full( BitStream &tgt )
{
    m_char.SendCharBuildInfo(tgt); //FIXEDOFFSET_pchar->character_Receive
    m_char.sendFullStats(tgt); //Entity::receiveFullStats(&FullStatsTokens, pak, FIXEDOFFSET_pchar, pkt_id_fullAttrDef, 1);
    sendBuffs(tgt); //FIXEDOFFSET_pchar->character_ReceiveBuffs(pak,0);
    bool has_sidekick=false;
    tgt.StoreBits(1,has_sidekick);
    if(has_sidekick)
    {
        bool is_mentor=false; // this flag might mean something totally different :)
        tgt.StoreBits(1,is_mentor);
        tgt.StorePackedBits(20,0); // sidekick partner db_id -> 10240
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

Entity::Entity()
{
    m_logout_sent = false;
    m_input_ack = 0;
    m_access_level = 9; // enables access to all deve
    field_60=0;
    field_64=0;
    field_78=0;
    m_state_mode_send=0;
    m_state_mode=0; // TODO: remove this later on, testing now.
    m_seq_update=0;
    m_has_titles=false;
    m_SG_info=false;
    pos = osg::Vec3(-60.5,180,0);
}

/*
PowerPool_Info Avatar::get_power_info( BitStream &src )
{
    PowerPool_Info res;
    res.id[0] = src.GetPackedBits(3);
    res.id[1] = src.GetPackedBits(3);
    res.id[2] = src.GetPackedBits(3);
    return res;
}
*/
/*
Avatar::Avatar(Entity *ent)
{
    m_ent = ent;
    m_full_options=false;
    m_first_person_view_toggle=false;
    m_player_collisions=0;
    m_options.mouse_invert=false;
    m_options.degrees_for_turns=1.0f;
    m_options.mouselook_scalefactor=1.0f;
    m_class_name = "Class_Blaster";
    m_origin_name= "Science";

}
*/
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
/*
void Avatar::send_character(BitStream &bs) const
{
    bs.StoreString(m_class_name); // class name
    bs.StoreString(m_origin_name); // origin name
    bs.StorePackedBits(5,0); // ?
    // powers/stats ?
    sendPowers(bs);
    sendPowers_main_tray(bs);
    sendBoosts(bs);
}
*/
//void Avatar::sendBuffs(BitStream &bs) const
//{
//    uint32_t num_buffs=0;
//    bs.StorePackedBits(5,num_buffs);
//    for(size_t idx=0; idx<num_buffs; ++idx)
//    {
//        sendPower(bs,0,0,0);
//    }
//}

void MapCostume::GetCostume( BitStream &src )
{
    this->m_costume_type = 1;
    m_body_type = src.GetPackedBits(3); // 0:male normal
    a = src.GetBits(32); // rgb ?

    m_height = src.GetFloat();
    m_physique = src.GetFloat();

    m_non_default_costme_p = src.GetBits(1);
    m_num_parts = src.GetPackedBits(4);
    for(int costume_part=0; costume_part<m_num_parts;costume_part++)
    {
        CostumePart part(m_non_default_costme_p);
        part.serializefrom(src);
        m_parts.push_back(part);
    }
}

void MapCostume::dump()
{
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Costume \n")));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    body type: 0x%08x\n"),m_body_type));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    a: 0x%08x\n"),a));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Height %f\n"),m_height));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Physique %f\n"),m_physique));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    ****** %d Parts *******\n"),m_num_parts));
    for(int i=0; i<m_num_parts; i++)
    {
        const CostumePart &cp(m_parts[i]);
        if(cp.m_full_part)
            ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%s,%s,%s,%s,0x%08x,0x%08x,%s,%s\n"),cp.m_geometry.c_str(),
            cp.m_texture_1.c_str(),cp.m_texture_2.c_str(),cp.name_3.c_str(),
            cp.m_colors[0],cp.m_colors[1],
            cp.name_4.c_str(),cp.name_5.c_str()
            ));
        else
            ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%s,%s,%s,%s,0x%08x,0x%08x,%s,%s\n"),cp.m_geometry.c_str(),
            cp.m_texture_1.c_str(),cp.m_texture_2.c_str(),cp.name_3.c_str(),
            cp.m_colors[0],cp.m_colors[1],
            cp.name_4.c_str(),cp.name_5.c_str()
            ));
    }
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    *************\n")));
}

void MapCostume::serializefrom( BitStream &src )
{
    GetCostume(src);
}
void MapCostume::serializeto( BitStream &bs ) const
{
    SendCommon(bs);
}
void MapCostume::SendCommon(BitStream &bs) const
{
    bs.StorePackedBits(3,m_body_type); // 0:male normal
    bs.StoreBits(32,a); // rgb ?

    bs.StoreFloat(m_height);
    bs.StoreFloat(m_physique);

    bs.StoreBits(1,m_non_default_costme_p);
    //m_num_parts = m_parts.size();
    assert(!m_parts.empty());
    bs.StorePackedBits(4,m_parts.size());
    for(int costume_part=0; costume_part<m_parts.size();costume_part++)
    {
        CostumePart part=m_parts[costume_part];
        // TODO: this is bad code, it's purpose is to NOT send all part strings if m_non_default_costme_p is false
        part.m_full_part = m_non_default_costme_p;
        part.serializeto(bs);
    }
}
