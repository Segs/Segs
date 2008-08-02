/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Entity.cpp 289 2006-10-03 14:52:25Z nemerle $
 */

#define _USE_MATH_DEFINES
#include <math.h>
#ifdef WIN32
#include "xmmintrin.h"
#else
#define _copysign(x,y) ((x)* ((y<0.0)? -1.0 : 1.0))
#endif
#include "Entity.h"
#include <limits>
#include <strstream>
float AngleDequantize(int value,int numb_bits)
{
	int max_val = 1<<numb_bits;
	float v = M_PI*((float)value/max_val) - M_PI/2;
	if(v<(-M_PI/2))
		return -1.0f;
	else if(v>(M_PI/2))
		return 1.0f;
	else if(v<0.00001)
		return 0.0f;
	return sinf(v);
}
u32 AngleQuantize(float value,int numb_bits)
{
	int max_val = 1<<numb_bits;
	float v = fabs(value)>1.0f ? _copysign(1.0f,value) : value ;
	v  = (asinf(v)+M_PI)/(2*M_PI); // maps -1..1 to 0..1
	v *= max_val;
//	assert(v<=max_val);
	return (u32)v;
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
				qrot.q[i] = fval;				
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
	u8 updates;
	updates = ((int)update_rot(0)) | (((int)update_rot(1))<<1) | (((int)update_rot(2))<<2);
	storeBitsConditional(bs,3,updates); //frank 7,0,0.1,0
	//NormalizeQuaternion(pEnt->qrot)
	// 
	//RestoreFourthQuatComponent(pEnt->qrot);
	for(int i=0; i<3; i++)
	{
		if(update_rot(i))
		{
			bs.StoreBits(9,AngleQuantize(qrot.q[i],9));	 // normalized quat, 4th param is recoverable from the first 3
		}
	}
}
static inline u32 quantize_float(float v)
{
	return u32(floorf(v*64)+0x800000);
}
void Entity::storePosition(BitStream &bs) const
{
//	float x = pos.vals.x;
	u32 packed;
	u32 diff; // changed bits are '1'
	bs.StoreBits(3,7); // frank -> 7,-60.5,0,180
	for(int i=0; i<3; i++)
	{
		packed = quantize_float(pos.v[i]);
		packed = packed<0xFFFFFF ? packed : 0xFFFFFF;
		diff = packed ^ prev_pos[i]; // changed bits are '1'
		bs.StoreBits(24,packed);
	}
}
void Entity::storePosUpdate(BitStream &bs) const
{
	storePosition(bs);
	// if(is_update)
	if(false)
	{
		bs.StoreBits(1,0); // not extra_info
	}
	storeOrientation(bs);
}
void Entity::storeUnknownBinTree(BitStream &bs) const
{
	bs.StoreBits(1,1);

}
void Entity::sendStateMode(BitStream &bs) const
{
	bs.StoreBits(1,m_state_mode_send); // no state mode
	if(m_state_mode_send)
	{
		storePackedBitsConditional(bs,3,m_state_mode);
	}
}
void Entity::sendSeqMoveUpdate(BitStream &bs) const
{
	bs.StoreBits(1,m_seq_update); // no seq update
	if(m_seq_update)
	{
		storePackedBitsConditional(bs,8,m_seq_upd_num1); // move index
		storePackedBitsConditional(bs,4,m_seq_upd_num2); //maxval is 255

	}
}
void Entity::sendSeqTriggeredMoves(BitStream &bs) const
{
	bs.StorePackedBits(1,0); // num moves
}
void Entity::sendNetFx(BitStream &bs) const
{
	bs.StorePackedBits(1,m_num_fx); // num fx
	//NetFx.serializeto();
	for(int i=0; i<m_num_fx; i++)
	{
		bs.StoreBits(8,m_fx1[i]); // net_id
		bs.StoreBits(32,m_fx2[i]); // command
		bs.StoreBits(1,0);
		storePackedBitsConditional(bs,10,0xCB8);
/*
		storeBitsConditional(bs,4,0);
		storePackedBitsConditional(bs,12,0xCB8);
		bs.StoreBits(1,0);
		if(false)
		{
			bs.StoreBits(32,0);
		}
*/
		storeBitsConditional(bs,4,0);
		storeBitsConditional(bs,32,0);
		storeFloatConditional(bs,0.0);
		storeFloatConditional(bs,10.0);
		storeBitsConditional(bs,4,10);
		storeBitsConditional(bs,32,0);
		int val=0;
		storeBitsConditional(bs,2,val);
		if(val==1)
		{
			bs.StoreFloat(0.0);
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
				storePackedBitsConditional(bs,8,0);
				bs.StorePackedBits(2,0);

			}
		}
		storeBitsConditional(bs,2,0);
		if(false)
		{
			bs.StoreFloat(0);
			bs.StoreFloat(0);
			bs.StoreFloat(0);
		}
		else
		{
			storePackedBitsConditional(bs,12,0x19b);
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
		ACE_ASSERT(false);
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

void PlayerEntity::sendCostumes(BitStream &bs) const
{
	storePackedBitsConditional(bs,2,m_costume_type);
	if(m_costume_type!=1)
	{
		ACE_ASSERT(false);
		return;
	}
	if(m_type==ENT_PLAYER)
	{
		bs.StoreBits(1,m_current_costume_set);
		if(m_current_costume_set)
		{
			bs.StoreBits(32,m_current_costume_idx);
			bs.StoreBits(32,m_num_costumes);
		}
		bs.StoreBits(1,m_multiple_costumes);
		if(m_multiple_costumes)
		{
			for(size_t idx=0; idx<=m_num_costumes; idx++)
			{
				m_costumes[idx]->serializeto(bs);
			}
		}
		else
		{
			m_costumes[m_current_costume_idx]->serializeto(bs);
		}
		//m_costume=m_costumes[m_current_costume_idx]; // this is only set if ent!=playerPtr
	}
	else
	{
		m_costume->serializeto(bs); // m_costume is the costume that is rendered
	}
	bs.StoreBits(1,m_supergroup_costume);
	if(m_supergroup_costume)
	{
		m_sg_costume->serializeto(bs);
		bs.StoreBits(1,m_using_sg_costume);
	}
}
void Entity::sendXLuency(BitStream &bs,float val) const
{
	int to_send = val*255;
	storeBitsConditional(bs,8,min(to_send,255));
}
void Entity::sendTitles(BitStream &bs) const
{
	bs.StoreBits(1,m_has_titles); // no titles
	if(m_has_titles)
	{
		if(m_type==ENT_PLAYER)
		{
			bs.StoreString(m_name);
			bs.StoreBits(1,0); // ent_player2->flag_F4
			storeStringConditional(bs,"");//max 32 chars
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
	bool is_logout=false;
	bs.StoreBits(1,is_logout);
	if(is_logout)
	{
		bs.StoreBits(1,0); // flags_1[1] set in entity 
		storePackedBitsConditional(bs,5,0); // time to logout, multiplied by 30
	}
}

void Entity::serializeto( BitStream &bs ) const
{
	// entity creation
	bs.StoreBits(1,m_create); // checkEntCreate_varD14
	bs.StoreBits(1,var_129C); // checkEntCreate_var_129C / ends creation destroys seq and returns NULL

	if(var_129C)
		return;
	bs.StorePackedBits(12,field_64);//  this will be put in  of created entity
	bs.StorePackedBits(2,m_type);
	if(m_type==ENT_PLAYER)
	{
		bs.StoreBits(1,m_create_player);
		if(m_create_player)
			bs.StorePackedBits(1,0x123); // var_1190: this will be put in field_C8 of created entity 
		bs.StorePackedBits(20,0);//bs.StorePackedBits(20,m_db_id);
	}
	else
	{
		bool val=false;
		bs.StoreBits(1,val);
		if(val)
		{
			bs.StorePackedBits(12,m_idx);
			bs.StorePackedBits(12,m_idx);
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
			bs.StoreBits(1,0);
			storeStringConditional(bs,"");
			storeStringConditional(bs,"");
			storeStringConditional(bs,"");
		}
		bs.StoreBits(1,m_hasname);
		if(m_hasname)
		{
			bs.StoreString(m_name);
		}
	}
	bs.StoreBits(1,0); //var_94 if set Entity.field_1818/field_1840=0 else field_1818/field_1840 = 255,2
	bs.StoreBits(32,field_60); // this will be put in field_60 of created entity 
	bs.StoreBits(1,m_hasgroup_name);
	if(m_hasgroup_name)
	{
		bs.StorePackedBits(2,0);// this will be put in field_1830 of created entity 
		bs.StoreString(m_name);
	}

	//if(m_classname_override)
	/*
	bs.StoreBits(1,false);
	if(false)
	{
	bs.StoreString(m_override_name);
	}
	*/
	// creation ends here
	bs.StoreBits(1,var_C); //var_C

	if(var_C)
	{
		bs.StoreBits(1,m_rare_bits);
	}
	if(m_rare_bits)
		sendStateMode(bs);
	storePosUpdate(bs);
	if(var_C)
	{
		sendSeqMoveUpdate(bs);
	}
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
		sendXLuency(bs,1.0f);
		sendTitles(bs);
	}
	if(m_pchar_things)
	{
		sendCharacterStats(bs);
		sendBuffs(bs);
		sendTargetUpdate(bs);
	}
	if(m_rare_bits)
	{
		sendOnOddSend(bs,m_odd_send);
		sendWhichSideOfTheForce(bs);
/*
		sendAllyID(bs);
		bs.StoreBits(1,m_is_villian);
		sendPvP(bs);
*/
		sendEntCollision(bs);
		sendNoDrawOnClient(bs);
/*
		sendContactOrPnpc(bs);
		bs.StoreBits(1,entReceiveAlwaysCon);
		bs.StoreBits(1,entReceiveSeeThroughWalls);
		sendPetName(bs);
*/
		sendAFK(bs);
		sendOtherSupergroupInfo(bs);
		sendLogoutUpdate(bs);
	}
}
void Entity::sendBuffs(BitStream &bs) const
{
	bs.StoreBits(1,0); // nothing here for now
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

void Avatar::GetCharBuildInfo( BitStream &src )
{
	src.GetString(m_class_name);
	src.GetString(m_origin_name);
	m_powers.push_back(get_power_info(src)); // primary_powerset power
	m_powers.push_back(get_power_info(src)); // secondary_powerset power
	m_trays.serializefrom(src);

}

void Avatar::DumpPowerPoolInfo( const PowerPool_Info &pool_info )
{
	for(int i=0; i<3; i++)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Pool_id[%d]: 0x%08x\n"),i,pool_info.id[i]));
	}
}

void Avatar::DumpBuildInfo()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    class: %s\n"),m_class_name.c_str()));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    origin: %s\n"),m_origin_name.c_str()));
	DumpPowerPoolInfo(m_powers[0]);
	DumpPowerPoolInfo(m_powers[1]);
}

void Entity::serializefrom_newchar( BitStream &src )
{
	int val = src.GetPackedBits(1); //2
	m_char.GetCharBuildInfo(src);
	m_costume = new MapCostume;
	m_costume->serializefrom(src);
	int t = src.GetBits(1); // The -> 1
	src.GetString(m_battle_cry);
	src.GetString(m_character_description);
}

void Entity::InsertUpdate( PosUpdate pup )
{
	m_update_idx++;
	m_update_idx %=64;
	m_pos_updates[m_update_idx]=pup;
}

void Entity::dump()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    //---------------Tray------------------\n")));
	m_char.m_trays.dump();
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    //---------------Costume---------------\n")));
	m_costume->dump();
}

Entity::Entity() : m_char(0)
{
	field_78=0;
	m_state_mode_send=0;
	m_state_mode=0;
	m_seq_update=0;
	m_has_titles=false;
}

PowerPool_Info Avatar::get_power_info( BitStream &src )
{
	PowerPool_Info res;
	res.id[0] = src.GetPackedBits(3);
	res.id[1] = src.GetPackedBits(3);
	res.id[2] = src.GetPackedBits(3);
	return res;
}
void Avatar::sendTray(BitStream &bs) const
{
	m_trays.serializeto(bs);
}
void Avatar::sendTrayMode(BitStream &bs) const
{
	bs.StoreBits(1,0);
	bs.StoreBits(1,1);
}
void Avatar::sendEntStrings(BitStream &bs) const
{
	bs.StoreString(m_ent->m_battle_cry); //max 128
	bs.StoreString(m_ent->m_character_description); //max 1024
}
void Avatar::sendWindow(BitStream &bs) const
{
	bs.StorePackedBits(1,0);
	bs.StorePackedBits(1,0);
	bs.StorePackedBits(1,0); // visible ?
	bs.StorePackedBits(1,0);
	bs.StorePackedBits(1,0);
	bs.StorePackedBits(1,0);
	bool a=false;
	bs.StoreBits(1,a);
	if(a)
	{
		bs.StorePackedBits(1,0);
		bs.StorePackedBits(1,0);
	}
	storeFloatConditional(bs,1.0f);
}
void Avatar::sendTeamBuffMode(BitStream &bs) const
{
	bs.StoreBits(1,0);
}
void Avatar::sendDockMode(BitStream &bs) const
{
	bs.StoreBits(32,0);
	bs.StoreBits(32,0);
}
void Avatar::sendChatSettings(BitStream &bs) const
{
	int i;
	bs.StorePackedBits(1,1);
	bs.StorePackedBits(1,2);
	bs.StorePackedBits(1,3);
	bs.StorePackedBits(1,4);
	bs.StorePackedBits(1,5);
	bs.StorePackedBits(1,6);
	for(i=0; i<5; i++)
	{
		bs.StorePackedBits(1,1);
		bs.StorePackedBits(1,2);
		bs.StorePackedBits(1,3);
		bs.StoreFloat(1.0f);
	}
	for(i=0; i<10; i++)
	{
		bs.StoreString("TestChat1");
		bs.StorePackedBits(1,1);
		bs.StorePackedBits(1,2);
		bs.StorePackedBits(1,3);
		bs.StorePackedBits(1,4);
		bs.StorePackedBits(1,5);
	}
	for(i=0; i<10; i++)
	{
		bs.StoreString("TestChat2");
		bs.StorePackedBits(1,1);
	}
}
void Avatar::sendDescription(BitStream &bs) const
{
	bs.StoreString("Desc1");
	bs.StoreString("Desc2");

}
void Avatar::sendTitles(BitStream &bs) const
{
	bs.StorePackedBits(5,0);
	bs.StoreString("Titles1");
	bs.StoreString("Titles2");
	bs.StoreString("Titles3");
	bs.StoreBits(32,0); //player type
	bs.StoreString("Titles4");
}
void Avatar::sendKeybinds(BitStream &bs) const
{
	bs.StoreString("Keybinds");
	for(int i=0; i<256; i++)
	{
		bs.StoreString(""); //w = +forward
		bs.StoreBits(32,0);
		bs.StoreBits(32,0);
	}

}
void Avatar::sendFriendList(BitStream &bs) const
{
	bs.StorePackedBits(1,0);
	bs.StorePackedBits(1,0);
}
void Avatar::serializeto(BitStream &bs) const
{
	u8 arr[16]={0};
	bs.StorePackedBits(1,0);
	bs.StorePackedBits(1,0);
	bs.StorePackedBits(2,0);
	bs.StorePackedBits(1,0);
	bs.StoreBitArray(arr,128);
	bs.StoreBits(1,0); //player type
	send_character(bs);
	Entity::sendAllyID(bs);
	Entity::sendPvP(bs);
	sendFullStats(bs);
	sendBuffs(bs);
	sendTray(bs);
	sendTrayMode(bs);
	bs.StoreString("Name"); // maxlength 32
	sendEntStrings(bs);
	for(int i=0; i<70; i++)
	{
		bs.StorePackedBits(1,70); // window index
		sendWindow(bs);
	}
	bs.StoreBits(10,0); // lfg
	bs.StoreBits(1,0); // super group mode
	bs.StoreBits(1,0); // pent->player_ppp.field_540
	bs.StoreBits(1,0); // pEnt->player_ppp.field_984C
	sendTeamBuffMode(bs);
	sendDockMode(bs);
	sendChatSettings(bs);
	sendTitles(bs);
	sendDescription(bs);
	bs.StoreString("Comment");
	sendKeybinds(bs);
	bs.StoreBits(1,m_full_options);
	if(m_full_options)
	{
		sendOptions(bs);
	}
	else
	{
		bs.StoreBits(1,m_options.mouse_invert);
		bs.StoreFloat(m_options.mouselook_scalefactor);
		bs.StoreFloat(m_options.degrees_for_turns);
	}
    for(int i=0; i<8; i++)
    {
        bs.StorePackedBits(9,0);
    }
	bs.StoreBits(1,m_first_person_view_toggle);
	bs.StoreBits(2,m_player_collisions);
	sendFriendList(bs);
	bs.StoreBits(1,0);
}
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
		bs.StorePackedBits(5,0);
		bs.StorePackedBits(4,0);
		sendPower(bs,0,0,0);
	}

}
void sendUnk1(BitStream &bs)
{
	bs.StorePackedBits(3,0); // count
	bs.StorePackedBits(3,0);
}
void sendUnk2(BitStream &bs)
{
	bs.StorePackedBits(5,0); // count
}
void sendUnk3(BitStream &bs) // inventory ?
{
	bs.StorePackedBits(3,0); // count
}
void Avatar::send_character(BitStream &bs) const
{
	bs.StoreString(m_class_name); // class name
	bs.StoreString(m_origin_name); // origin name
	bs.StorePackedBits(5,0); // ?
	// powers/stats ?
	sendPowers(bs);
	sendUnk1(bs);
	sendUnk2(bs);
	sendUnk3(bs);	
}
void Avatar::sendFullStats(BitStream &bs) const
{
	bs.StoreBits(1,1);
	bs.StorePackedBits(1,0);
	{
		// CurrentAttributes
		bs.StoreBits(1,1);
		bs.StorePackedBits(1,0); // hitpoints
			bs.StorePackedBits(6,100*10);
		bs.StoreBits(1,0);

	}
	bs.StoreBits(1,0);
}
void Avatar::sendBuffs(BitStream &bs) const
{
	bs.StorePackedBits(5,0);
}
void Avatar::sendOptions(BitStream &) const
{
    ACE_ASSERT(!"Not implemented yet\n");
}



void MapCostume::GetCostume( BitStream &src )
{
	this->m_costume_type = 1;
	m_body_type = src.GetPackedBits(3); // 0:male normal
	a = src.GetBits(32); // rgb ?

	split.m_height = src.GetFloat();
	split.m_physique = src.GetFloat();

	m_non_default_costme_p = src.GetBits(1);
	m_num_parts = src.GetPackedBits(4);
	for(int costume_part=0; costume_part<m_num_parts;costume_part++)
	{
		CostumePart part;
		GetCostumeString_Cached(src,part.name_0);
		GetCostumeString_Cached(src,part.name_1);
		GetCostumeString_Cached(src,part.name_2);
		part.m_colors[0] = GetCostumeColor_Cached(src);
		part.m_colors[1] = GetCostumeColor_Cached(src);
		if(m_non_default_costme_p)
		{
			GetCostumeString(src,part.name_3);
			GetCostumeString(src,part.name_4);
			GetCostumeString(src,part.name_5);
		}
		m_parts.push_back(part);
	}
}

void MapCostume::dump()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Costume \n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    body type: 0x%08x\n"),m_body_type));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    a: 0x%08x\n"),a));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Height %f\n"),split.m_height));			
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Physique %f\n"),split.m_physique));			
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    ****** %d Parts *******\n"),m_num_parts));		
	for(int i=0; i<m_num_parts; i++)
	{
		if(m_parts[i].m_generic)
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%s,%s,%s,%s,0x%08x,0x%08x,%s,%s\n"),m_parts[i].name_0.c_str(),
			m_parts[i].name_1.c_str(),m_parts[i].name_2.c_str(),m_parts[i].name_3.c_str(),
			m_parts[i].m_colors[0],m_parts[i].m_colors[1],
			m_parts[i].name_4.c_str(),m_parts[i].name_5.c_str()
			));		
		else
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%s,%s,%s,%s,0x%08x,0x%08x,0x%08x,0x%08x,%s,%s\n"),m_parts[i].name_0.c_str(),
			m_parts[i].name_1.c_str(),m_parts[i].name_2.c_str(),m_parts[i].name_3.c_str(),
			m_parts[i].m_colors[0],m_parts[i].m_colors[1],m_parts[i].m_colors[2],m_parts[i].m_colors[3],
			m_parts[i].name_4.c_str(),m_parts[i].name_5.c_str()
			));		
	}
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    *************\n")));
}

void MapCostume::GetCostumeString_Cached( BitStream &src,string &tgt )
{
	strstream strm;
	bool in_cache= src.GetBits(1);
	if(in_cache)
	{
		if(stringcache.size()==1)
		{
			tgt = stringcache[0];
			return;
		}
		int in_cache_idx = src.GetPackedBits(12);
		strm <<"Hash:"<<in_cache_idx<<"\0";
		tgt=strm.str();
		tgt.push_back(0);
		//tgt = stringcache[in_cache_idx];
	}
	else
	{
		src.GetString(tgt);
		stringcache.push_back(tgt);
	}
}

void MapCostume::GetCostumeString( BitStream &src,string &tgt )
{
	int name_in_hash= src.GetBits(1);
	if(name_in_hash)
	{
		int part_idx=0;
		//if(hash_map.find(name)!=hash_map.end())
		//{
		//	var_4C = 1;
		//	part_idx = hash_map[name];
		//}
		//GetHashTableMaxIndexBits() const 12
		char buf[128]={0};
		part_idx=src.GetPackedBits(12);
		sprintf(buf,"0x%08x",part_idx);
		buf[9]=0;
		tgt= buf;
	}
	else
		src.GetString(tgt);
}

u32 MapCostume::GetCostumeColor( BitStream &src )
{
	bool in_hash= src.GetBits(1);
	if(in_hash)
	{
		u16 hash_key =src.GetBits(9);
		return 0xFFFFFFFF; 
	}
	else
	{
		return src.GetBits(32);
	}
}

u32 MapCostume::GetCostumeColor_Cached( BitStream &src )
{
	bool in_cache= src.GetBits(1);
	u32 tgt;
	if(in_cache)
	{
		if(colorcache.size()==1)
		{
			tgt = colorcache[0];
			return tgt;
		}
		int in_cache_idx = src.GetPackedBits(10);
		tgt = 0xBAD00000|in_cache_idx;
	}
	else
	{
		tgt = src.GetBits(32);
		colorcache.push_back(tgt);
	}
	return tgt;
}

void MapCostume::clear_cache()
{
	stringcachecount_bitlength=0;
	colorcachecount_bitlength=0;
	stringcache.clear();
	colorcache.clear();
}

void MapCostume::serializefrom( BitStream &src )
{
	GetCostume(src);
}
void MapCostume::serializeto( BitStream &bs ) const
{
	SendCommon(bs);
}
class FullCostume : public MapCostume
{
public:
	void serializeto( BitStream &bs ) const
	{
		storePackedBitsConditional(bs,2,1);
		bs.StoreBits(1,0);
		if(false)
		{
		}
		bs.StoreBits(1,0);
	}
};
void MapCostume::SendCommon(BitStream &bs) const
{
	bs.StorePackedBits(3,m_body_type); // 0:male normal
	bs.StoreBits(32,a); // rgb ?

	bs.StoreFloat(split.m_height);
	bs.StoreFloat(split.m_physique);

	bs.StoreBits(1,m_non_default_costme_p);
	bs.StorePackedBits(4,m_num_parts);
	for(int costume_part=0; costume_part<m_num_parts;costume_part++)
	{
		CostumePart part=m_parts[costume_part];
		SendCostumeString_Cached(bs,part.name_0);
		SendCostumeString_Cached(bs,part.name_1);
		SendCostumeString_Cached(bs,part.name_2);
		SendCostumeColor_Cached(bs,part.m_colors[0]);
		SendCostumeColor_Cached(bs,part.m_colors[1]);
		if(m_non_default_costme_p)
		{
			SendCostumeString_Cached(bs,part.name_0);
			SendCostumeString_Cached(bs,part.name_1);
			SendCostumeString_Cached(bs,part.name_2);
		}
	}
}

void MapCostume::SendCostumeString_Cached( BitStream &tgt,const string &src ) const
{
	int cache_idx=0;
#ifdef HASH_LOADING
	cache_idx=cache_lookup(color);
#endif
	tgt.StoreBits(1,(cache_idx||src.size()==0));
	if(cache_idx||src.size()==0)
	{
		tgt.StorePackedBits(12,cache_idx);
	}
	else
	{
		tgt.StoreString(src);
	}
}

void MapCostume::SendCostumeColor_Cached( BitStream &tgt,u32 color ) const
{
	int cache_idx=0;
#ifdef HASH_LOADING
	cache_idx=cache_lookup(color);
#endif
	tgt.StoreBits(1,(cache_idx||color==0));
	if(cache_idx||color==0)
	{
		tgt.StorePackedBits(10,cache_idx);
	}
	else
	{
		tgt.StoreBits(32,color);
	}
}