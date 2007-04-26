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
#include "MapPacket.h"
#include <limits>
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
		storePackedBitsConditional(bs,8,m_seq_upd_num1);
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
		bs.StoreBits(2,m_fx1[i]); // net_id
		bs.StoreBits(32,m_fx2[i]); // command
		bs.StoreBits(1,0);
		storeBitsConditional(bs,4,0);
		storePackedBitsConditional(bs,12,0xCB8);
		bs.StoreBits(1,0);
		if(false)
		{
			bs.StoreBits(32,0);
		}
		storeBitsConditional(bs,4,0);
		storeBitsConditional(bs,32,0);
		storeFloatConditional(bs,0.0);
		storeFloatConditional(bs,10.0);
		storeBitsConditional(bs,4,10);
		storeBitsConditional(bs,32,0);
		storeBitsConditional(bs,2,10);
		if(false)
		{
			bs.StoreFloat(0);
			bs.StoreFloat(0);
			bs.StoreFloat(0);
		}
		else
		{
			storePackedBitsConditional(bs,8,0);
			bs.StorePackedBits(2,0);
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
			bs.StorePackedBits(2,0);
		}

	}
}
void Entity::sendCostume(BitStream &bs) const
{
	m_costume.serializeto(bs);
}
void Entity::sendXLuency(BitStream &bs,float val) const
{
	int to_send = val*255;
	storeBitsConditional(bs,8,min(to_send,255));
}
void Entity::sendTitles(BitStream &bs) const
{
	bs.StoreBits(1,0); // no titles
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
			storeStringConditional(bs,"");

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
			// ...
		}
	}
}
void Entity::sendLogoutUpdate(BitStream &bs) const
{
	bool is_logout=false;
	bs.StoreBits(1,is_logout);
	if(is_logout)
	{
		bs.StoreBits(1,0);
		storePackedBitsConditional(bs,5,0);
	}
}
void pktMap_Server_EntitiesResp::sendControlState(BitStream &bs) const
{
	bs.StoreBits(1,0); // sendFuturePushList
	sendServerPhysicsPositions(bs);
	sendServerControlState(bs);
}
void pktMap_Server_EntitiesResp::sendServerPhysicsPositions(BitStream &bs) const
{
	bs.StoreBits(1,0); 
	bs.StoreBits(1,0); 
	bs.StoreBits(1,0); 
}
void pktMap_Server_EntitiesResp::sendServerControlState(BitStream &bs) const
{
	bs.StoreBits(1,0); 
	bs.StoreBits(1,0); 
}
void pktMap_Server_EntitiesResp::storePowerInfoUpdate(BitStream &bs) const
{
	bs.StoreBits(1,0);
	bs.StoreBits(1,0);
}
void pktMap_Server_EntitiesResp::storePowerModeUpdate(BitStream &bs) const
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
void pktMap_Server_EntitiesResp::storeBadgeUpdate(BitStream &bs) const
{
	bs.StoreBits(1,0);
	bs.StoreBits(1,0);
}
void pktMap_Server_EntitiesResp::storeGenericinventoryUpdate(BitStream &bs)const
{
	bs.StorePackedBits(1,0);
}
void pktMap_Server_EntitiesResp::storeInventionUpdate(BitStream &bs)const
{
	bs.StorePackedBits(1,0);
}
void pktMap_Server_EntitiesResp::storeTeamList(BitStream &bs) const
{
	bs.StoreBits(1,0);
}
void pktMap_Server_EntitiesResp::storeSuperStats(BitStream &bs) const
{
	bs.StorePackedBits(1,0);
}
void pktMap_Server_EntitiesResp::storeGroupDyn(BitStream &bs) const
{
	bs.StorePackedBits(1,0);
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
	bs.StoreString(m_ent->ent_string1); //max 128
	bs.StoreString(m_ent->ent_string2); //max 1024
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
