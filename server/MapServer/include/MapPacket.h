/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: MapPacket.h 313 2007-01-25 11:21:43Z nemerle $
 */

#pragma once
#include <string>
#include <list>
#include <limits>
#include <ace/OS_NS_strings.h>

#include "PacketBase.h"
#include "opcodes/ControlCodes.h"
#include "opcodes/Opcodes.h"

#include "MapClient.h"
#include "CommonNetStructures.h"

using namespace std;
class NetCommand
{
	float normalizedCircumferenceToFloat(int number,int numbits)
	{
		// something like this : ((number*3.141592)/(1<<numbits))-3.141592
		return 0.0f;
	}
public:

	struct Argument
	{
		int type;
		void *targetvar;
	};
	NetCommand(int acl,const std::string &name,vector<Argument> &args):m_arguments(args)
	{
		m_required_access_level=acl;
		m_name=name;
	}
	int serializefrom(BitStream &bs)
	{
		for(size_t i=0; i<m_arguments.size(); i++)
		{
			switch(m_arguments[i].type)
			{
			case 1:
				{
					int res=bs.GetPackedBits(1);
					if(m_arguments[i].targetvar)
						*((int *)m_arguments[i].targetvar) = res;
					ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("CommRecv %s:arg%d : %d\n"),m_name.c_str(),i,res));
					break;
				}
			case 2:
			case 4:
				{
					std::string res;
					bs.GetString(res); // postprocessed
					ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("CommRecv %s:arg%d : %s\n"),m_name.c_str(),i,res.c_str()));
					break;
				}
			case 3:
				{
					float res = bs.GetFloat();
					ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("CommRecv %s:arg%d : %f\n"),m_name.c_str(),i,res));
					break;
				}
			case 5:
				{
					float res1 = normalizedCircumferenceToFloat(bs.GetBits(14),14); 
					ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("CommRecv %s:arg%d : %s\n"),m_name.c_str(),i,res1));
					break;
				}
			case 6:
				break;
			case 7:
				{
					float res1 = bs.GetFloat();
					float res2 = bs.GetFloat();
					float res3 = bs.GetFloat();
					ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("CommRecv %s:arg%d : %f,%f,%f\n"),m_name.c_str(),i,res1,res2,res3));
					break;
				}
			}
		}
		return 1;
	}
	int clientside_idx;
	int m_required_access_level;
	std::string m_name;
	vector<Argument> m_arguments;

};
class NetCommandManager
{
	std::map<std::string,NetCommand *> m_name_to_command;
	vector<NetCommand *> m_commands_level0;
	void SendCommandShortcutsWorker(MapClient *client,BitStream &tgt,const vector<NetCommand *> &commands,const vector<NetCommand *> &commands2);
public:
	void SendCommandShortcuts(MapClient *client,BitStream &tgt,const vector<NetCommand *> &commands2);
	NetCommand * getCommandByName(const std::string &name)
	{
		return m_name_to_command[name];
	}
	void addCommand(NetCommand *cmd)
	{
		ACE_ASSERT(m_name_to_command.find(cmd->m_name)==m_name_to_command.end());
		m_name_to_command[cmd->m_name]=cmd;
		m_commands_level0.push_back(cmd);
	}
};
typedef ACE_Singleton<NetCommandManager,ACE_Thread_Mutex> NetCommandManagerSingleton; // AdminServer Interface 

class PowerPool_Info
{
public:
	int id[3];
};

class Power
{
public:
	int entry_type;
	int unkn1,unkn2;
	string sunkn1;
	string sunkn2;
	string sunkn3;
	void serializeto(BitStream &tgt) const;
	void serializefrom(BitStream &src)
	{
		entry_type = src.GetBits(4);
		switch(entry_type)
		{
		case 1:
			unkn1 = src.GetBits(32);
			unkn2 = src.GetBits(32);
			break;
		case 2:
			unkn1 = src.GetPackedBits(3);
			unkn2 = src.GetPackedBits(3);
			break;
		case 6:
		case 12:
			src.GetString(sunkn1);
			src.GetString(sunkn2);
			src.GetString(sunkn3);
			break;
		case 0:
			break;
		default:
			ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Unknown tray entry type %d\n"),entry_type));
		}
	}
	void Dump()
	{
		switch(entry_type)
		{
		case 1: case 2:
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("[(0x%x,0x%x)]"),unkn1,unkn2));
			break;
		case 6: case 12:
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("[(%s,%s,%s)]"),sunkn1.c_str(),sunkn2.c_str(),sunkn3.c_str()));
			break;
		case 0:
			break;
		default:
			ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Unknown tray entry type %d\n"),entry_type));
		}

	}
};
class PowerTray
{
public:
	int unkn0;
	Power m_powers[10];
	Power *getPower(size_t idx)
	{
		if(idx<10)
			return &m_powers[idx];
		return NULL;
	}
	int setPowers()
	{
		int res=0;
		for(int i=0; i<10; i++)
		{
			res += (m_powers[i].entry_type!=0);
		}
		return res;
	}
	void serializefrom(BitStream &src)
	{
		for(int power_slot=0; power_slot<10; power_slot++)
			m_powers[power_slot].serializefrom(src);
	}
	void serializeto(BitStream &tgt) const
	{
		for(int power_slot=0; power_slot<10; power_slot++)
			m_powers[power_slot].serializeto(tgt);
	}
	void Dump()
	{
		for(int power_slot=0; power_slot<10; power_slot++)
		{
			m_powers[power_slot].Dump();
		}

	}
};
class PowerTrayGroup
{
	static const int num_trays=2; // was 3, displayed trays
	PowerTray m_trays[9];
	u32 m_power_rel1,m_power_rel2;
	bool m_c;
public:
	PowerTrayGroup()
	{
		m_power_rel1=m_power_rel2=0;
		m_c=false;
	}
	void serializeto(BitStream &tgt) const
	{
		for(int i=0; i<num_trays; i++)
			tgt.StoreBits(32,m_trays[i].unkn0);
		for(int bar_num=0; bar_num<9; bar_num++)
			m_trays[bar_num].serializeto(tgt);
		bool m_c = false;
		tgt.StoreBits(1,m_c);
		if(m_c)
		{
			tgt.StoreBits(32,m_power_rel1);
			tgt.StoreBits(32,m_power_rel2);
		}
	}
	void serializefrom(BitStream &src)
	{
		for(int i=0; i<num_trays; i++)
		{
			m_trays[i].unkn0 = src.GetBits(32);
		}
		for(int bar_num=0; bar_num<9; bar_num++)
			m_trays[bar_num].serializefrom(src);
		m_c = src.GetBits(1);
		if(m_c)
		{
			m_power_rel1= src.GetBits(32);
			m_power_rel2= src.GetBits(32);
		}
	}
	void dump()
	{
		for(int i=0; i<3; i++)
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Tray[%d].unkn1: 0x%08x\n"),i,m_trays[i].unkn0));
		}
		for(int bar_num=0; bar_num<9; bar_num++)
		{
			if(m_trays[bar_num].setPowers()==0)
				continue;
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Tray %d ***"),bar_num));
			m_trays[bar_num].Dump();
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("***\n")));
		}
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_c %d\n"),m_c));
		if(m_c)
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_power_rel1 0x%08x\n"),m_power_rel1));
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_power_rel2 0x%08x\n"),m_power_rel2));
		}
	}

};

class MapCostume : Costume
{
	vector<string> stringcache;
	u32 stringcachecount_bitlength;
	vector<u32> colorcache;
	u32 colorcachecount_bitlength;
	bool costume_sends_nonquantized_floats;

public:
	~MapCostume(){}
	union
	{
		struct{
			float m_height;
			float m_physique;
		} split;
		float m_floats[30];

	};

	u8 m_costume_type;
	int costume_type_idx_P;
	int costume_sub_idx_P;
	MapCostume()
	{
		stringcachecount_bitlength=0;
		colorcachecount_bitlength=0;
	}
	void clear_cache()
	{
		stringcachecount_bitlength=0;
		colorcachecount_bitlength=0;
		stringcache.clear();
		colorcache.clear();
	}
	void serializefrom(BitStream &)
	{
		ACE_ASSERT(!"still not implemented");
	}
	void serializeto(BitStream &bs) const;
	void GetCostumeString(BitStream &src,string &tgt);
	void GetCostumeString_Cached(BitStream &src,string &tgt);
	u32 GetCostumeColor(BitStream &src)
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
	u32 GetCostumeColor_Cached(BitStream &src)
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
			int in_cache_idx = src.GetBits(colorcachecount_bitlength);
			tgt = colorcache[in_cache_idx];
		}
		else
		{
			tgt = GetCostumeColor(src);
			size_t max_color_idx =(size_t(1)<<colorcachecount_bitlength)-1; 
			if(colorcache.size()>max_color_idx)
			{
				colorcachecount_bitlength++;
			}
			colorcache.push_back(tgt);
		}
		return tgt;
	}

	void GetCostume(BitStream &src);

	void dump();
};
class ClientOptions
{
public:
	bool mouse_invert;
	float mouselook_scalefactor;
	float degrees_for_turns;
};
class PosUpdate
{
public:
	Vector3 posvec;
	Quaternion quat;
	int a;
	int b;
};
class Entity;
class Avatar : public NetStructure //! not to be confuzzled with GameServer's Character :)
{
	bool m_full_options;
	ClientOptions m_options;
	bool m_first_person_view_toggle;
	u8 m_player_collisions;
public:
	Entity *m_ent;
	u32 field_5A8;
	u32 field_498;
	std::string char_arr_10_5AC; 
	vector<PowerPool_Info> m_powers;
	PowerTrayGroup m_trays;
	std::string m_class_name;
	std::string m_origin_name;
	virtual ~Avatar()
	{
	    m_ent = NULL; // we only borrowed this pointer	    
	}
	Avatar(Entity *m_ent);
	virtual void serializeto(BitStream &bs) const;
	virtual void serializefrom(BitStream &){ACE_ASSERT(false);};

	void send_character(BitStream &bs)const;
	void sendFullStats(BitStream &bs)const;
	void sendBuffs(BitStream &bs)const;
	void sendTray(BitStream &bs)const;
	void sendTrayMode(BitStream &bs)const;
	void sendEntStrings(BitStream &bs)const;
	void sendWindow(BitStream &bs)const;
	void sendTeamBuffMode(BitStream &bs)const;
	void sendDockMode(BitStream &bs)const;
	void sendChatSettings(BitStream &bs)const;
	void sendTitles(BitStream &bs)const;
	void sendDescription(BitStream &bs)const;
	void sendKeybinds(BitStream &bs)const;
	void sendOptions(BitStream &bs)const;
	void sendFriendList(BitStream &bs)const;

};
class Entity : public NetStructure
{
public:	
	int field_64;
	int field_60;
	int field_68;
	int field_78;
	int m_num_titles;
	int m_num_fx;
	vector<u8> m_fx1;
	vector<u32> m_fx2;
	vector<u8> m_fx3;
	int m_state_mode;
	bool m_state_mode_send;
	Entity() : m_char(0)
	{
		field_78=0;
	}
	virtual ~Entity(){}
	void dump()
	{
		m_char.m_trays.dump();
		m_costume.dump();
	}
	u32 getIdx() const {return m_idx;}
	virtual void serializeto(BitStream &bs)const
	{
		// entity creation
		bs.StoreBits(1,m_create); // checkEntCreate_varD14
		bs.StoreBits(1,var_129C); // checkEntCreate_var_129C / ends creation end returns NULL
		if(var_129C)
			return;
		bs.StorePackedBits(12,field_64);//  this will be put in  of created entity
		bs.StorePackedBits(2,m_type);
		if(m_type==2)
		{
			bs.StoreBits(1,m_create_player);
			if(m_create_player)
				bs.StorePackedBits(1,0x123); // var_1190: this will be put in field_C8 of created entity 
			bs.StorePackedBits(22,field_68); //var_90: this will be put in field_68 of created entity 
			bs.StorePackedBits(1,m_player_villain); //this will be put in player_ppp[985C]
			bs.StorePackedBits(1,m_origin_idx);
			bs.StorePackedBits(1,m_class_idx);
			bs.StoreBits(1,m_selector1);
			if(m_selector1)
			{	
				bs.StoreBits(1,0);
				bs.StorePackedBits(5,m_num_titles);
				storeStringConditional(bs,"");
				storeStringConditional(bs,"");
				storeStringConditional(bs,"");
				bs.StoreBits(32,0);
				storeStringConditional(bs,"");
			}

		}
		else
		{
			bs.StoreBits(1,0);
			if(false)
			{
				
			}
			if(m_create_player)
			{

			}
			else
			{

			}
		}
		bs.StoreBits(1,m_hasname);
		if(m_hasname)
			bs.StoreString(m_name);
		bs.StoreBits(1,0); //var_94 if set Entity.field_1818/field_1840=0 else field_1818/field_1840 = 255,2
		bs.StoreBits(32,field_60); // this will be put in field_60 of created entity 
		bs.StoreBits(1,m_hasgroup_name);
		if(m_hasgroup_name)
		{
			bs.StorePackedBits(2,0);// this will be put in field_1830 of created entity 
			bs.StoreString(m_name);
		}
		//if(m_classname_override)
		bs.StoreBits(1,false);
		if(false)
		{
			bs.StoreString(m_override_name);
		}
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
			if(m_rare_bits)
				sendSeqTriggeredMoves(bs);
		}
		// NPC -> m_pchar_things=0 ?
		bs.StoreBits(1,m_pchar_things);
		if(m_pchar_things)
		{
			sendNetFx(bs);
		}
		if(m_rare_bits)
		{
			sendCostume(bs);
			sendXLuency(bs,1.0f);
			sendTitles(bs);
		}
		if(m_hasRagdoll)
		{
			sendRagDoll(bs);
		}
		else
		{
			//			sendRagDollNull(bs);
		}
		if(m_pchar_things)
		{
			//sendCharacterStats(bs);
			//sendBuffs(bs);
			//sendTargetUpdate(bs);
			//sendStatusEffects(bs);
		}
		if(m_rare_bits)
		{
			sendOnOddSend(bs,m_odd_send);
			//bs.StoreBits(1,0);//
			sendAllyID(bs);
			bs.StoreBits(1,m_is_villian);
			sendPvP(bs);
			sendEntCollision(bs);
			sendNoDrawOnClient(bs);
			sendContactOrPnpc(bs);
			bs.StoreBits(1,entReceiveAlwaysCon);
			bs.StoreBits(1,entReceiveSeeThroughWalls);
			sendPetName(bs);
			sendAFK(bs);
			sendOtherSupergroupInfo(bs);
			sendLogoutUpdate(bs);
		}
	}
	bool m_odd_send;
	bool m_SG_info;
	void sendStateMode(BitStream &bs) const;
	void sendOnOddSend(BitStream &bs,bool is_odd=true) const;

	void sendSeqMoveUpdate(BitStream &bs) const;
	void sendSeqTriggeredMoves(BitStream &bs) const;
	void sendNetFx(BitStream &bs) const;
	void sendCostume(BitStream &bs) const;
	void sendXLuency(BitStream &bs,float xluency) const;
	void sendTitles(BitStream &bs) const;
	void sendRagDoll(BitStream &bs) const;
	static void sendAllyID(BitStream &bs);
	static void sendPvP(BitStream &bs);
	void sendEntCollision(BitStream &bs) const;
	void sendNoDrawOnClient(BitStream &bs)const; 
	void sendContactOrPnpc(BitStream &bs)const; 
	void sendPetName(BitStream &bs)const; 
	void sendAFK(BitStream &bs)const; 
	void sendOtherSupergroupInfo(BitStream &bs)const; 
	void sendLogoutUpdate(BitStream &bs)const; 

	void storePosition(BitStream &bs) const;	
	void storeOrientation(BitStream &bs) const;
	void storeUnknownBinTree(BitStream &bs) const;
	void storePosUpdate(BitStream &bs) const;	
	int  getOrientation(BitStream &bs);
	virtual void serializefrom(BitStream &){};
	bool update_rot(int axis) const // returns true if given axis needs updating
	{
		if(axis==axis)
			return true;
		return false;
	}
	PowerPool_Info GetPowerInfo(BitStream &src)
	{
		PowerPool_Info res;
		res.id[0] = src.GetPackedBits(3);
		res.id[1] = src.GetPackedBits(3);
		res.id[2] = src.GetPackedBits(3);
		return res;
	}
	void GetCharBuildInfo(BitStream &src)
	{
		src.GetString(m_char.m_class_name);
		src.GetString(m_char.m_origin_name);
		m_char.m_powers.push_back(GetPowerInfo(src)); // primary_powerset power
		m_char.m_powers.push_back(GetPowerInfo(src)); // secondary_powerset power
	}
	void DumpPowerPoolInfo(const PowerPool_Info &pool_info)
	{
		for(int i=0; i<3; i++)
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Pool_id[%d]: 0x%08x\n"),i,pool_info.id[i]));
		}
	}
	void DumpBuildInfo()
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    class: %s\n"),m_char.m_class_name.c_str()));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    origin: %s\n"),m_char.m_origin_name.c_str()));
		DumpPowerPoolInfo(m_char.m_powers[0]);
		DumpPowerPoolInfo(m_char.m_powers[1]);
	}

	void serializefrom_newchar(BitStream &src)
	{
		src.GetPackedBits(1); //2
		//player_type = src.GetBits(1); // player_type hero/villain ?
		GetCharBuildInfo(src);
		m_char.m_trays.serializefrom(src);
		m_costume.GetCostume(src);
		int t = src.GetBits(1);
		src.GetString(ent_string1);
		src.GetString(ent_string2);
	}
	void InsertUpdate(PosUpdate pup)
	{
		m_update_idx++;
		m_update_idx %=64;
		m_pos_updates[m_update_idx]=pup;
	}
	bool m_seq_update;
	bool m_is_villian;
	bool m_contact;
	int m_seq_upd_num1;
	int m_seq_upd_num2;
	PosUpdate m_pos_updates[64];
	size_t m_update_idx;
	string ent_string1;
	string ent_string2;
	u32 field_E30;
	bool var_B4;
	MapCostume m_costume;
	Avatar m_char;
	bool entReceiveAlwaysCon;
	bool entReceiveSeeThroughWalls;
	int pkt_id_QrotUpdateVal[3];
	Quaternion qrot;
	Vector3 pos;
	u32 prev_pos[3];
	bool m_selector1,m_pchar_things,var_C          ,m_hasname  ,m_hasgroup_name,m_classname_override;
	bool m_create   ,m_hasRagdoll  ,m_create_player,m_rare_bits;
	int current_client_packet_id;
	string m_name, m_group_name, m_override_name;
	u8 m_origin_idx,m_class_idx;
	u8 m_type;
	u32 m_idx;
	u32 var_8C;
	u32 var_90;
	bool player_type;
	bool m_player_villain;
	u32 var_1190;
	bool var_129C;
};
#if 0
class DoorEntity : public Entity
{
public:
	void serializeto(BitStream &bs)
	{
		// entity creation
		bs.StoreBits(1,m_create); // checkEntCreate_varD14
		bs.StoreBits(1,var_129C); // checkEntCreate_var_129C / ends creation end returns NULL
		if(var_129C)
			return;
		bs.StorePackedBits(12,field_64);
		bs.StorePackedBits(2,8);
		bs.StoreBits(1,0);
		if(false)
		{

		}
		if(m_create_player)
		{

		}
		else
		{

		}
		bs.StoreBits(1,1);
		bs.StoreString("Dr");
		bs.StoreBits(1,0); //var_94 if set Entity.field_1818/field_1840=0 else field_1818/field_1840 = 255,2
		bs.StoreBits(32,field_60); // this will be put in field_60 of created entity 
		bs.StoreBits(1,0);//m_hasgroup_name
		bs.StoreBits(1,false); //if(m_classname_override)
		// creation ends here
		bs.StoreBits(1,true); //var_C
		bs.StoreBits(1,1); // m_rare_bits
		sendStateMode(bs);
		storePosUpdate(bs);
		sendSeqMoveUpdate(bs);
		sendSeqTriggeredMoves(bs);
		bs.StoreBits(1,0); //m_pchar_things
		sendCostume(bs);
		sendXLuency(bs,1.0f);
		sendTitles(bs);
		sendRagDoll(bs);
		bs.StoreBits(1,0);//sendOnOddSend(bs,m_odd_send);
		sendAllyID(bs);
		bs.StoreBits(1,m_is_villian);
		sendPvP(bs);
		sendEntCollision(bs);
		sendNoDrawOnClient(bs);
		sendContactOrPnpc(bs);
		bs.StoreBits(1,0);
		bs.StoreBits(1,0);
		sendPetName(bs);
		sendAFK(bs);
		sendOtherSupergroupInfo(bs);
		sendLogoutUpdate(bs);
	}
};
#endif

class pktCS_SendEntity : public GamePacket
{
protected:
	void DumpEntity()
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_unkn1 0x%08x\n"),m_unkn1));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_b %d\n"),m_b));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    //------------Build Info-------\n")));
		if(m_ent)
			m_ent->DumpBuildInfo();
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    //---------------Tray----------\n")));
		if(m_ent)
			m_ent->dump();
	}
	virtual void dependent_dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IMap:pktCS_SendEntity\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_cookie 0x%08x\n"),m_cookie));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_city_of_developers %d\n"),m_city_of_developers));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_new_character %d\n"),m_new_character));
		if(m_new_character)
		{
			DumpEntity();
		}
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
	bool IsCostumePartName_NotSet(string &str)
	{
		return ((str.length()==0) || (ACE_OS::strcasecmp(str.c_str(),"None")==0));
	}
public:
	pktCS_SendEntity()
	{
		m_unkn1=0;
		m_opcode=9;//after this packet it expects opcode 5
		arr_size=0;
	}
	virtual void serializefrom(BitStream &src)
	{
		m_cookie = src.GetPackedBits(1);
		//m_city_of_developers = src.GetBits(1);
		m_new_character=src.GetBits(1);
		if(m_new_character)
		{
			m_ent = new Entity; //EntityManager::CreatePlayer();
			m_ent->serializefrom_newchar(src);
		}
		arr_size = src.GetReadableBits()>>3;
		if(arr_size>512)
			arr_size = 512;
		src.GetBitArray((u8*)arr,arr_size<<3);
	}
	virtual void serializeto(BitStream &tgt) const
	{
		tgt.StorePackedBits(1,m_cookie);
		tgt.StoreBits(1,m_city_of_developers);
		tgt.StoreBits(1,m_new_character);
		if(m_new_character)
		{
			tgt.StorePackedBits(1,m_unkn1);
			tgt.StoreBits(1,m_unkn2);
			tgt.StoreString(m_sunkn1);
			tgt.StoreString(m_sunkn2);
		}
	}
public:
	Entity *m_ent;
	u32 m_power_rel1,m_power_rel2;
	u8 arr[512];
	size_t arr_size;
	u32 m_cookie;
	bool m_city_of_developers;
	bool m_new_character,m_b,m_c,m_d;
	u32 m_unkn1;
	u32 m_unkn2;
	u32 m_unkn3;
	u32 m_unkn4;
	u32 m_unkn5;
	std::string m_sunkn1;
	std::string m_sunkn2;
	std::string m_sunkn3;

};
class pktSC_Connect : public GamePacket
{
protected:
	virtual void dependent_dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IMap:pktSC_Connect\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_resp 0x%08x\n"),m_resp));
		if(m_resp==0)
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_fatal_error %s\n"),m_fatal_error.c_str()));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	pktSC_Connect()
	{
		m_opcode=5;
	}
	virtual void serializefrom(BitStream &src)
	{
		m_resp = src.GetPackedBits(1);
		if(m_resp==0)
			src.GetString(m_fatal_error);
		else 
			m_unkn1 = src.GetFloat();
	}
	virtual void serializeto(BitStream &tgt)const
	{
		tgt.StorePackedBits(1,m_resp);
		if(m_resp==0)
			tgt.StoreString(m_fatal_error);
		else
			tgt.StoreFloat(m_unkn1);
	}
	u32 m_resp;
	float m_unkn1;
	std::string m_fatal_error;
};
class pktCS_RequestShortcuts : public GamePacket
{
protected:
	virtual void dependent_dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IMap:pktCS_RequestShortcuts\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	pktCS_RequestShortcuts()
	{
		m_opcode=4;
	}
	virtual void serializefrom(BitStream &){}
	virtual void serializeto(BitStream &) const {}
};
class pktSC_CmdShortcuts : public GamePacket
{
protected:
	virtual void dependent_dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IMap:pktSC_CmdShortcuts\n%I{\n")));
/*
		for(size_t i=0; i<m_commands.size(); i++)
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    command shortcut %d:%s\n"),i,m_commands[i]->m_name.c_str()));
		}
*/
		for(size_t i=0; i<m_shortcuts2.size(); i++)
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_shortcuts2[%d] %s\n"),i,m_shortcuts2[i].c_str()));
		}
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	pktSC_CmdShortcuts()
	{
		m_opcode=4;
		m_client=0;
	}
	virtual void serializefrom(BitStream &src);
	virtual void serializeto(BitStream &tgt) const;
/*
	void PushCommand(NetCommand *c)
	{
		m_commands.push_back(c);
	}
*/
	u32 m_num_shortcuts2;
	//vector<NetCommand *> m_commands;
	vector<NetCommand *> m_commands2;  // m_commands2 will get filled after we know more about them
	vector<std::string>  m_shortcuts2;
	MapClient *m_client;
};

class pktCS_RequestScene : public GamePacket
{
protected:
	virtual void dependent_dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IpktMap_Client_RequestScene\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_resp 0x%08x\n"),unkn1));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	pktCS_RequestScene()
	{
		m_opcode=3;
	}
	virtual void serializefrom(BitStream &src)
	{
		unkn1 = src.GetBits(1);
	}
	virtual void serializeto(BitStream &tgt) const
	{
		tgt.StorePackedBits(1,unkn1);
	}
	bool unkn1;
};
class pktMap_Client_RequestEntities : public GamePacket
{
protected:
	virtual void dependent_dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IpktMap_Client_RequestEntities\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	pktMap_Client_RequestEntities()
	{
		m_opcode=5;
	}
	virtual void serializefrom(BitStream &) {}
	virtual void serializeto(BitStream &) const {}
};

class MapRef : public NetStructure
{
public:
	int m_idx;
	Matrix4x3 m_matrix;
	string m_name;
	MapRef(int idx,Matrix4x3 &mat,string name) : m_idx(idx),m_matrix(mat),m_name(name){}
	MapRef(): m_idx(0),m_name(""){}
	void dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    idx %d\n"),m_idx));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    name %s\n"),m_name.c_str()));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    |%f,%f,%f|\n"),m_matrix.row1.v));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    |%f,%f,%f|\n"),m_matrix.row2.v));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    |%f,%f,%f|\n"),m_matrix.row3.v));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    |%f,%f,%f|\n"),m_matrix.row4.v));
	}
	void serializefrom(BitStream &src)
	{
		m_idx = src.GetPackedBits(1);
		if(m_idx<0) return;
		src.GetString(m_name);
		recvTransformMatrix(src,m_matrix);
	}
	void serializeto(BitStream &tgt) const
	{
		tgt.StorePackedBits(1,m_idx);
		tgt.StoreString(m_name);
		SendTransformMatrix(tgt,m_matrix);
	}

};
class pktMap_Server_SceneResp : public GamePacket
{
protected:
	virtual void dependent_dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IpktMap_Server_SceneResp\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    undos_PP 0x%08x\n"),undos_PP));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    var_14 0x%08x\n"),var_14));
		if(var_14)
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_outdoor_map 0x%08x\n"),m_outdoor_map));
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_map_number 0x%08x\n"),m_map_number));
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn1 0x%08x\n"),unkn1));
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_map_desc %s\n"),m_map_desc.c_str()));
		}
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    flag_dword_151D5D8 0x%08x\n"),current_map_flags));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    num_base_elems 0x%08x\n"),num_base_elems));
		for(size_t i=0; i<num_base_elems; i++)
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn2 0x%08x\n"),unkn2));
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_trays[i] %s\n"),m_trays[i].c_str()));
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_crc[i] 0x%08x\n"),m_crc[i]));
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn2 0x%08x\n"),unkn2));
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn1 0x%08x\n"),unkn1));
		}
		if(num_base_elems!=0)
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn2 0x%08x\n"),unkn2));
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_trays[0] %s\n"),m_trays[0].c_str()));
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_crc[0] 0x%08x\n"),m_crc[0]));
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn2 0x%08x\n"),unkn2));
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn1 0x%08x\n"),unkn1));
		}
		for(size_t i=0; i<m_refs.size(); i++)
		{
			m_refs[i].dump();
		}
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	pktMap_Server_SceneResp()
	{
		m_opcode=6;
	}
	void getGrpElem(BitStream &src,int idx)
	{
		if(src.GetBits(1))
		{
			src.GetString(m_trays[idx]);
			m_crc[idx] = src.GetBits(32);
			if(!src.GetBits(1))
			{
				int var_14=0;
				int var_10;
				while(var_10=src.GetPackedBits(1)-1>=0)
				{
//					if(!var_14)
//						var_14=syb_599950(var_4);
					groupnetrecv_5927C0(src,var_10,var_14);
				}
			}
			else
				reqWorldUpdateIfPak(src);
		}
	}
	void reqWorldUpdateIfPak(BitStream &src)
	{
		//src.GetBits(1);
		ACE_ASSERT(0);
	}
	void groupnetrecv_5927C0(BitStream &src,int a,int b)
	{
		if(!src.GetBits(1))
			return;
		ACE_ASSERT(0);
		string def_filename;
		src.GetString(def_filename);
	}

	virtual void serializefrom(BitStream &src)
	{
		unkn1=false;
		bool IAmAnArtist;
		undos_PP = src.GetPackedBits(1);
		var_14 = src.GetBits(1);
		if(var_14)
		{
			m_outdoor_map = src.GetBits(1);
			m_map_number = src.GetPackedBits(1);
			unkn1 = src.GetPackedBits(1);
			if(unkn1)
			{
				IAmAnArtist=false;
				//IAmAnArtist=isDevelopmentMode())
			}
			src.GetString(m_map_desc);
		}
		current_map_flags = src.GetBits(1); // is beaconized
		num_base_elems = src.GetPackedBits(1);
		m_crc.resize(num_base_elems);
		m_trays.resize(num_base_elems);
		for(size_t i=1; i<num_base_elems; i++)
		{
			getGrpElem(src,i);
		}
		if(num_base_elems!=0)
		{
			getGrpElem(src,0);
		}
		MapRef r;
		do
		{
			r.serializefrom(src);
			m_refs.push_back(r);
		} while(r.m_idx>=0);
		src.GetPackedBits(1); //unused
		ref_count = src.GetPackedBits(1);
		ACE_ASSERT(ref_count==m_refs.size());
		src.GetBits(32); //unused - crc ?
		ref_crc=src.GetBits(32); // 0x3f6057cf
	}
	virtual void serializeto(BitStream &tgt) const;
	string m_map_desc;
	int ref_count;
	int ref_crc;
	int var_4;
	bool m_outdoor_map;
	bool current_map_flags;
	size_t num_base_elems;
	int undos_PP;
	bool var_14;
	vector<string> m_trays;
	vector<u32> m_crc;
	vector<MapRef> m_refs;
	int unkn1; 
	int m_map_number;
	bool unkn2;
};
class pktMap_Server_EntitiesResp : public GamePacket
{
protected:
	virtual void dependent_dump(void);
	MapClient *m_client;
public:
	pktMap_Server_EntitiesResp(MapClient *cl)
	{
		m_opcode=3;
		m_client = cl;
		abs_time=db_time=0;

	}
	virtual void serializefrom(BitStream &) 
	{
	}
	void sendCommands(BitStream &tgt) const
	{
		tgt.StorePackedBits(1,m_num_commands);
	}
	virtual void serializeto(BitStream &tgt) const;
	void sendServerPhysicsPositions(BitStream &bs) const;
	void sendControlState(BitStream &bs) const;
	void sendServerControlState(BitStream &bs) const;
	void storePowerInfoUpdate(BitStream &bs) const;
	void storePowerModeUpdate(BitStream &bs) const;
	void storeBadgeUpdate(BitStream &bs) const;
	void storeGenericinventoryUpdate(BitStream &bs) const;
	void storeInventionUpdate(BitStream &bs) const;
	void storeTeamList(BitStream &bs) const;
	void storeSuperStats(BitStream &bs) const;
	void storeGroupDyn(BitStream &bs) const;

	bool entReceiveUpdate;
	bool unkn1;
	bool unkn2;
	bool debug_info;
	bool selector1;
	u32 abs_time;
	u32 db_time;
	u32 u1;
	u32 u2;
	u32 u3;
	u16 m_debug_idx;
	u8 dword_A655C0;
	u8 BinTrees_PPP;
	u32 m_num_commands;
	u32 m_command_idx[15];
	string m_commands[15];
	u32 m_num_commands2;
	string m_commands2[15];
};
class pktMap_Character_6 : public GamePacket
{
protected:
	virtual void dependent_dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IpktMap_Character_6\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    mapserver_cookie 0x%08x\n"),mapserver_cookie));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    console_enabled 0x%08x\n"),console_enabled));
		for(size_t i=0; i<16; i++)
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    final_password_hash[%d]:0x%08x;\n"),i,final_password_hash[i]));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	pktMap_Character_6()
	{
		m_opcode=6;
	}
	virtual void serializefrom(BitStream &src) 
	{
		mapserver_cookie = src.GetPackedBits(1);
		console_enabled = src.GetPackedBits(1);
		src.GetBitArray(final_password_hash,128);
	}
	virtual void serializeto(BitStream &tgt) const
	{
		tgt.StorePackedBits(1,mapserver_cookie);
		tgt.StorePackedBits(1,console_enabled);
		tgt.StoreBitArray(final_password_hash,128);
	}
	u32 mapserver_cookie;
	u32 console_enabled;
	u8 final_password_hash[16];
};
class pktMap_Client_InputState : public GamePacket
{
protected:
	virtual void dependent_dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IpktMap_Client_InputState\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    contain_control_update 0x%08x\n"),contain_control_update));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    has_target 0x%08x\n"),has_target));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    target_idx 0x%08x\n"),target_idx));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    input_send 0x%08x\n"),input_send));
		if(input_send) {
		}
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	pktMap_Client_InputState()
	{
		m_opcode=2;
	}
	virtual void serializefrom(BitStream &src)
	{
		contain_control_update = src.GetBits(1);
		has_target = src.GetBits(1);
		target_idx = src.GetPackedBits(14);
		input_send = src.GetBits(1);
		if(input_send) {
		}
	}
	virtual void serializeto(BitStream &tgt) const
	{
		tgt.StoreBits(1,contain_control_update);
		tgt.StoreBits(1,has_target);
		tgt.StorePackedBits(14,target_idx);
		tgt.StoreBits(1,input_send);
		if(input_send) {
		}
	}
	bool contain_control_update;
	bool has_target;
	bool input_send;
	u32 target_idx;
};

class MapPacketFactory : public ControlPacketFactory
{
public:
	virtual GamePacket *PacketFromStream(u8 opcode,BitStream &bs) const
	{
		GamePacket *res = NULL;
		switch(opcode)
		{
		case 0:
			return ControlPacketFactory::PacketFromStream(opcode,bs);
		case COMM_CONNECT:
			res = new pktCS_Connect; break;
		case 2:
			res = new pktMap_Client_InputState; break;
		case 3:
			res = new pktCS_RequestScene; break;
		case 4:
			res = new pktCS_RequestShortcuts; break;
		case 5:
			res = new pktMap_Client_RequestEntities; break;
		case 6:
			res = new pktMap_Character_6; break;
		case 9:
			res = new pktCS_SendEntity; break;
		default:
			res = new UnknownGamePacket; break;
		}
		if(res)
		{
			res->m_opcode=opcode;
		}
		return res;
	}
};
