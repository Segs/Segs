/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: MapPacket.cpp $
 */

#include "MapHandler.h"
#include "MapPacket.h"
#include "GameProtocol.h"
#include "PacketCodec.h"
#include "ServerManager.h"
#include "MapClient.h"
#include "SEGSMap.h"
void pktMap_Server_EntitiesResp::serializeto( BitStream &tgt ) const
{
	tgt.StoreBits(1,entReceiveUpdate);
	sendCommands(tgt);
	tgt.StoreBits(32,abs_time);
	tgt.StoreBits(32,db_time);

	tgt.StoreBits(1,unkn2);
	if(unkn2==0) 
	{
		tgt.StoreBits(1,debug_info);
		if(selector1==0) 
		{
			tgt.StoreBits(2,dword_A655C0);
			tgt.StoreBits(2,BinTrees_PPP);
		}
	}

	//else debug_info = false;
	m_client->getCurrentMap()->m_entities.sendEntities(tgt);
	if(debug_info&&!unkn2) 
	{
		m_client->getCurrentMap()->m_entities.sendDebuggedEntities(tgt);
		m_client->getCurrentMap()->m_entities.sendGlobalEntDebugInfo(tgt);
	}
	sendControlState(tgt);
	m_client->getCurrentMap()->m_entities.sendDeletes(tgt);
	m_client->getCharEntity()->m_char.serializeto(tgt);
	storePowerInfoUpdate(tgt);
	storePowerModeUpdate(tgt);
	storeBadgeUpdate(tgt);
	storeGenericinventoryUpdate(tgt);
	storeInventionUpdate(tgt);
	storeTeamList(tgt);
	storeSuperStats(tgt);
	storeGroupDyn(tgt);
	bool additional=false;
	tgt.StoreBits(1,additional);
	if(additional)
	{
		tgt.StoreFloat(0.0f);
		tgt.StoreFloat(0.0f); // camera_yaw
		tgt.StoreFloat(0.0f);
	}
}

void pktMap_Server_EntitiesResp::dependent_dump( void )
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IpktMap_Server_EntitiesResp\n%I{\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    entReceiveUpdate 0x%08x\n"),entReceiveUpdate));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_command_idx 0x%08x\n"),m_command_idx));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    abs_time 0x%08x\n"),abs_time));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    db_time 0x%08x\n"),db_time));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn2 0x%08x\n"),unkn2));
	if(unkn2==0) 
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    debug_info 0x%08x\n"),debug_info));
		if(selector1==0) 
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    dword_A655C0 0x%08x\n"),dword_A655C0));
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    BinTrees_PPP 0x%08x\n"),BinTrees_PPP));
		}
	}
	if(debug_info) 
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_debug_idx 0x%08x\n"),m_debug_idx));
	}
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn2 0x%08x\n"),unkn2));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

void NetCommandManager::SendCommandShortcutsWorker( MapClient *client,BitStream &tgt,const vector<NetCommand *> &commands,const vector<NetCommand *> &commands2 )
{
	if(commands.size()==0)
	{
		tgt.StorePackedBits(1,~0);//0xFFFFFFFF
	}
	else
	{
		for(u32 i=0; i<(u32)commands.size(); i++)
		{
			tgt.StorePackedBits(1,i+1);
			tgt.StoreString(commands[i]->m_name);
			client->AddShortcut(i,commands[i]);
		}
	}
	tgt.StorePackedBits(1,(u32)commands2.size());
	if(commands2.size()>0)
	{
		for(u32 i=0; i<(u32)commands2.size(); i++)
		{
			tgt.StoreString(commands2[i]->m_name);
		}
	}
}

void NetCommandManager::SendCommandShortcuts( MapClient *client,BitStream &tgt,const vector<NetCommand *> &commands2 )
{
	switch(client->getAccessLevel())
	{
	case 0:
	case 1:
		SendCommandShortcutsWorker(client,tgt,m_commands_level0,commands2); break;
	default:
		ACE_ASSERT(false);
	}
}

void pktSC_CmdShortcuts::serializefrom( BitStream &src )
{
	u32 i=0;
	string read;
	while(0!=(i = src.GetPackedBits(1)))
	{
		src.GetString(read);
		ACE_ASSERT(NULL!=NetCommandManagerSingleton::instance()->getCommandByName(read));
		m_client->AddShortcut(i,NetCommandManagerSingleton::instance()->getCommandByName(read));
		//m_commands.push_back(NetCommandManagerSingleton::instance()->getCommandByName(read));
	}
	i = src.GetPackedBits(1);
	if(i==0)
		return;
	for(size_t j=0; j<i; j++)
	{
		src.GetString(read);
		m_shortcuts2.push_back(read);
	}
}

void pktSC_CmdShortcuts::serializeto( BitStream &tgt ) const
{
	// command shortcuts sent to the client depend on it's access level
	NetCommandManagerSingleton::instance()->SendCommandShortcuts(m_client,tgt,m_commands2);
}

void MapCostume::GetCostume( BitStream &src )
{
	//body_type = src.GetPackedBits(3);
	//face_bits = src.GetBits(32);
	float c = src.GetFloat();
	float d = src.GetFloat();
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
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    a: 0x%08x\n"),a));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    b: 0x%08x\n"),b));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    c: 0x%08x\n"),c));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Height %f\n"),split.m_height));			
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Physique %f\n"),split.m_physique));			
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Floats (")));
	for(int i=2; i<29; i++)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT (" %f "),m_floats[i]));			
	}
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT (")\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    ****** Parts *******\n")));		
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
	bool in_cache= src.GetBits(1);
	if(in_cache)
	{
		if(stringcache.size()==1)
		{
			tgt = stringcache[0];
			return;
		}
		int in_cache_idx = src.GetBits(stringcachecount_bitlength);
		tgt = stringcache[in_cache_idx];
	}
	else
	{
		GetCostumeString(src,tgt);
		size_t max_string_idx =(size_t(1)<<stringcachecount_bitlength)-1; 
		if(stringcache.size()>max_string_idx)
		{
			stringcachecount_bitlength++;
		}
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
		char buf[128];
		part_idx=src.GetPackedBits(12);
		sprintf(buf,"0x%08x",part_idx);
		tgt= buf;
	}
	else
		src.GetString(tgt);
}

void MapCostume::serializeto( BitStream &bs ) const
{
	storePackedBitsConditional(bs,2,m_costume_type);
	switch(m_costume_type)	
	{
	case 1: // full costume
		{
			bs.StoreBits(1,0);
			if(false)
			{
			}
			bs.StoreBits(1,0);
		}
	case 2: // npc costume
		bs.StorePackedBits(12,costume_type_idx_P); // npc number
		bs.StorePackedBits(1,costume_sub_idx_P); // costume idx ?
		storeStringConditional(bs,"");
		break;
	}
}

void Power::serializeto( BitStream &tgt ) const
{
	tgt.StoreBits(4,entry_type);
	switch(entry_type)
	{
	case 1:
		tgt.StoreBits(32,unkn1); //powersetIdx
		tgt.StoreBits(32,unkn2);
		break;
	case 2:
		tgt.StorePackedBits(3,unkn1);
		tgt.StorePackedBits(3,unkn2);
		break;
	case 6:
	case 12:
		tgt.StoreString(sunkn1);
		tgt.StoreString(sunkn2);
		tgt.StoreString(sunkn3);
		break;
	case 0:
		break;
	default:
		ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Unknown tray entry type %d\n"),entry_type));
	}
}

void pktMap_Server_SceneResp::serializeto( BitStream &tgt ) const
{
	tgt.StorePackedBits(1,undos_PP);
	tgt.StoreBits(1,var_14);
	if(var_14)
	{
		tgt.StoreString(m_map_desc);
		tgt.StoreBits(1,m_outdoor_map);
		tgt.StorePackedBits(1,m_map_number);
		//tgt.StorePackedBits(1,unkn1);
	}
	//tgt.StoreBits(1,current_map_flags);
	tgt.StorePackedBits(1,m_trays.size());
	u32 hashes[] = {0x00000000,0xAFD34459,0xE63A2B76,0xFBBAD9D4,
		0x9AE0A9D4,0x06BDEF70,0xA47A21F8,0x5FBF835D,
		0xFF25F3F6,0x70E6C422,0xF1CCC459,0xCBD35A55,
		0x64CCCC31,0x535B08CC};
	for(size_t i=1; i<m_trays.size(); i++)
	{
		//			ACE_TRACE(!"Hold yer horses!");
		tgt.StoreBits(1,1);
		// old coh use it that way, first string of the 2 is optional (path), second is mandatory
		//NetStructure::storeStringConditional(tgt,m_trays[i]);
		//tgt.StoreString("");
		NetStructure::storeStringConditional(tgt,"");
		tgt.StoreString(m_trays[i]);
		if(i<12)
			tgt.StoreBits(32,hashes[i]); // crc ? 
		else
			tgt.StoreBits(32,0); // crc ? 
		//tgt.StoreBits(1,0);
		tgt.StorePackedBits(1,0);
		tgt.StorePackedBits(1,0);
	}
	if(m_trays.size()!=0)
	{
		tgt.StoreBits(1,1);
		NetStructure::storeStringConditional(tgt,"");
		tgt.StoreString(m_trays[0]);
		//tgt.StoreString(m_trays[0]);
		tgt.StoreBits(32,hashes[0]); // crc ?
		//tgt.StoreBits(1,0);
		tgt.StorePackedBits(1,0);
		tgt.StorePackedBits(1,0);
	}
	for(size_t i=0; i<m_refs.size(); i++)
	{
		m_refs[i].serializeto(tgt);
	}
	tgt.StorePackedBits(1,~0); // finishing marker,-1
	tgt.StorePackedBits(1,0xD8); //unused
	tgt.StorePackedBits(1,ref_count);
	tgt.StoreBits(32,0); //unused - crc ?
	tgt.StoreBits(32,ref_crc); // 0x3f6057cf
}