/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: MapPacket.h 313 2007-01-25 11:21:43Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef MAPPACKET_H
#define MAPPACKET_H

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


class Entity;
class PlayerEntity;
class pktCS_SendEntity : public GamePacket
{
protected:
		BitStream m_newchar_optional; // this is a copy of the packet used to recover new character.
		void	DumpEntity();
virtual void	dependent_dump(void);
		bool	IsCostumePartName_NotSet(string &str);
public:
					pktCS_SendEntity();
virtual void		serializefrom(BitStream &src);
virtual void		serializeto(BitStream &tgt) const;
		Entity *	get_new_character();
public:
		Entity *	m_ent;
		u32			m_power_rel1,m_power_rel2;
		u32			m_cookie;
		bool		m_city_of_developers;
		bool		m_new_character,m_b,m_c,m_d;
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
	}
	virtual void serializeto(BitStream &tgt)const
	{
		tgt.StorePackedBits(1,m_resp);
		if(m_resp==0)
			tgt.StoreString(m_fatal_error);
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
class pktCS_UIWindowState : public GamePacket
{
protected:
	virtual void dependent_dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IpktCS_UIWindowState\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    window_idx 0x%08x\n"),window_idx));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    color_PP 0x%08x\n"),color_PP));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    field_0 0x%08x\n"),field_0));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    field_10 0x%08x\n"),field_10));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    field_14 0x%08x\n"),field_14));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    field_18 0x%08x\n"),field_18));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    field_20 0x%08x\n"),field_20));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    field_24 0x%08x\n"),field_24));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    field_4 0x%08x\n"),field_4));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    field_8 0x%08x\n"),field_8));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    field_C 0x%08x\n"),field_C));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
	size_t color_PP;
	size_t field_0;
	bool field_10;
	size_t field_14;
	int field_18;
	size_t field_20;
	size_t field_24;
	size_t field_4;
	size_t field_8;
	size_t field_C;
	size_t window_idx;
public:
	pktCS_UIWindowState()
	{
		m_opcode=14;
		field_8=field_C=0;
	}
	virtual void serializefrom(BitStream &src)
	{
		window_idx=src.GetPackedBits(1);
		field_0=src.GetPackedBits(1); //width
		field_4=src.GetPackedBits(1); //height
		size_t res=src.GetPackedBits(1);
		if(res==4)
			field_18 = 2;
		field_24=res;
		field_14=src.GetPackedBits(1);
		color_PP=src.GetPackedBits(1);
		field_20=src.GetPackedBits(1);
		bool has_field_10=src.GetBits(1);
		field_10=has_field_10=has_field_10;
		if ( has_field_10 )
		{
			field_8=src.GetPackedBits(1);
			field_C=src.GetPackedBits(1);
		}
	}
	virtual void serializeto(BitStream &tgt) const
	{
/*
		tgt.StorePackedBits(1, window_idx);
		tgt.StorePackedBits(1, a2->field_0);
		tgt.StorePackedBits(1, a2->field_4);
		if ( a2->field_18 == 2 )
			tgt.StorePackedBits(1, 4);
		else
			tgt.StorePackedBits(1, a2->field_24);
		tgt.StorePackedBits(1, a2->field_14);
		tgt.StorePackedBits(1, a2->color_PP);
		tgt.StorePackedBits(1, a2->field_20);
		if ( a2->field_10 )
		{
			tgt.StoreBits(1, 1);
			tgt.StorePackedBits(1, a2->field_8);
			result = tgt.StorePackedBits(1, a2->field_C);
		}
		else
		{
			result = tgt.StoreBits(1, 0);
		}
*/
	}
};
class pktCS_RequestEntities : public GamePacket
{
protected:
	virtual void dependent_dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IpktCS_RequestEntities\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	pktCS_RequestEntities()
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
	bool reduced_transform;
	Matrix4x3 m_matrix;
	TransformStruct m_transforms;
	string m_name;
	MapRef(int idx,Matrix4x3 &mat,string name) : m_idx(idx),m_matrix(mat),m_name(name){reduced_transform=false;}
	MapRef(int idx,string name,Vector3 &pos,Vector3 &rot) : m_idx(idx),m_name(name)
	{
		reduced_transform=true;
		m_transforms=TransformStruct(pos,rot,Vector3(),true,true,false);
	}
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
		tgt.StorePackedBits(16,m_idx); //def_id
		//tgt.StoreString(m_name);
		if(reduced_transform)
		{
			SendTransformMatrix(tgt,m_transforms);
		}
		else
		{
			SendTransformMatrix(tgt,m_matrix);
		}

	}

};
class pktSC_SceneResp : public GamePacket
{
protected:
	virtual void dependent_dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IpktSC_SceneResp\n%I{\n")));
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
	pktSC_SceneResp()
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
class pktSC_EntitiesResp : public GamePacket
{
protected:
	virtual void dependent_dump(void);
	MapClient *m_client;
public:
	pktSC_EntitiesResp(MapClient *cl,bool t1)
	{
		m_opcode=t1 ? 2 : 3;
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
class pktCS_Character_6 : public GamePacket
{
protected:
	virtual void dependent_dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IpktCS_Character_6\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    mapserver_cookie 0x%08x\n"),mapserver_cookie));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    console_enabled 0x%08x\n"),console_enabled));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	pktCS_Character_6()
	{
		m_opcode=6;
	}
	virtual void serializefrom(BitStream &src) 
	{
		mapserver_cookie = src.GetPackedBits(1);
		console_enabled = src.GetPackedBits(1);
	}
	virtual void serializeto(BitStream &tgt) const
	{
		tgt.StorePackedBits(1,mapserver_cookie);
		tgt.StorePackedBits(1,console_enabled);
	}
	u32 mapserver_cookie;
	u32 console_enabled;
};
class pktCS_InputState : public GamePacket
{
protected:
	virtual void dependent_dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IpktCS_InputState\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    contain_control_update 0x%08x\n"),contain_control_update));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    has_target 0x%08x\n"),has_target));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    target_idx 0x%08x\n"),target_idx));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	pktCS_InputState()
	{
		m_opcode=2;
	}
	virtual void serializefrom(BitStream &src)
	{
		contain_control_update = src.GetBits(1);
		if(contain_control_update)
		{
			s8 csc_delta_bits = src.GetBits(5);
			s16 a3 = src.GetBits(16); // count of some sort ?
			//if v3
				rev403720(a3, src, csc_delta_bits); 
			rev403720(a3, src, csc_delta_bits);
		}
		has_target = src.GetBits(1);
		target_idx = src.GetPackedBits(14);
		while(src.GetBits(1))
		{
			ACE_ASSERT(!"Not Implemented");

		};
		u8 opt_idx;
		// options set
		while(opt_idx = src.GetPackedBits(1))
		{
			ACE_ASSERT(!"Not Implemented");
		}
		//what follows is a bitstream of many packets that are concatenated into this packet 
		// but it is put as an bitarray so it's byte aligned
		// TODO: Check if any other packet is followed by g_pak
		src.ByteAlign(true,false);
	}

	void rev403720( s16 a3, BitStream &src, s8 csc_delta_bits )
	{
		bool only_first=false;
		while(a3--)
		{
			u32 a2_filed_4=8;
			if(src.GetBits(1)==0)
				a2_filed_4 = src.GetBits(4);
			if(src.GetBits(1)==0)
				src.GetBits(csc_delta_bits);
			else
				src.GetBits(2);
			switch(a2_filed_4)
			{
			case 0: case 1: case 2: case 3:
			case 4: case 5:
				//pakSendBits(a1, 1, val_18 & 1);
				src.GetBits(1);
				break;
			case 6:
			case 7:
				//pakSendBits(a1, 11, val_18 & 0x7FF);
				src.GetBits(11);
				break;
			case 8:
				src.GetBits(1);
				if(only_first)
				{
					src.GetPackedBits(8); // val_18 - x
					src.GetPackedBits(8); // val_1C - y

				}
				else
				{
					only_first=true;
					src.GetBits(32); // val_18
					src.GetPackedBits(10); // val_18 - val_1C
				}
				// x = val_18
				// y = val_1C
				if(src.GetBits(1))
				{
					src.GetBits(8); // field_20
				}
				break;
			case 9:
				src.GetBits(8);
				break;
			case 10:
				src.GetBits(1);
				break;
			default:
				ACE_ASSERT(0);
				break;
			}
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
			res = new pktCS_InputState; break;
		case 3:
			res = new pktCS_RequestScene; break;
		case 4:
			res = new pktCS_RequestShortcuts; break;
		case 5:
			res = new pktCS_RequestEntities; break;
		case 6:
			res = new pktCS_Character_6; break;
		case 9:
			res = new pktCS_SendEntity; break;
		case 14:
			res = new pktCS_UIWindowState; break;
			
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

#endif // MAPPACKET_H



