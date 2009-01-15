/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: MapPacket.cpp $
 */
#include <strstream>

#include "MapHandler.h"
#include "MapPacket.h"
#include "GameProtocol.h"
#include "PacketCodec.h"
#include "ServerManager.h"
#include "MapClient.h"
#include "SEGSMap.h"
#include "Entity.h"
void pktMap_Server_EntitiesResp::serializeto( BitStream &tgt ) const
{
	tgt.StoreBits(1,entReceiveUpdate);
	sendCommands(tgt);

	tgt.StoreBits(32,abs_time);
	//tgt.StoreBits(32,db_time);

	tgt.StoreBits(1,unkn2);
	if(unkn2==0) 
	{
		tgt.StoreBits(1,debug_info);
		tgt.StoreBits(1,selector1);
		if(selector1==1) 
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
	sendServerPhysicsPositions(tgt);
	sendControlState(tgt);
	m_client->getCurrentMap()->m_entities.sendDeletes(tgt);
	if(m_opcode==3)
	{
		m_client->getCharEntity()->m_char.serializeto(tgt);
	}
	else
	{
		m_client->getCharEntity()->m_char.sendFullStats(tgt);
	}
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
		if(selector1==1) 
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
	//tgt.StorePackedBits(1,m_trays.size());
	u32 hashes[] = {0x00000000,0xAFD34459,0xE63A2B76,0xFBBAD9D4,
		0x9AE0A9D4,0x06BDEF70,0xA47A21F8,0x5FBF835D,
		0xFF25F3F6,0x70E6C422,0xF1CCC459,0xCBD35A55,
		0x64CCCC31,0x535B08CC};

	//			ACE_TRACE(!"Hold yer horses!");
	// overriding defs
	tgt.StorePackedBits(1,-1); // finisher
	// overriding groups
	tgt.StorePackedBits(1,-1); // fake it all the way
	return;
	if(0)
	{
		for(size_t i=1; i<m_refs.size(); i++)
		{
			//			ACE_TRACE(!"Hold yer horses!");
			tgt.StorePackedBits(1,0); // next element idx 
			tgt.StoreBits(1,0);
			if(0)
			{
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
		}
		tgt.StorePackedBits(1,-1); // finisher
	}
	if(0)
	{
		for(size_t i=0; i<m_refs.size(); i++)
		{
			tgt.StorePackedBits(1,0); // next element idx 
			m_refs[i].serializeto(tgt);
		}
	}
	tgt.StorePackedBits(1,~0); // finishing marker,-1
	tgt.StorePackedBits(1,0xD8); //unused
	tgt.StorePackedBits(1,ref_count);
	tgt.StoreBits(32,0); //unused - crc ?
	tgt.StoreBits(32,ref_crc); // 0x3f6057cf
}

pktCS_SendEntity::pktCS_SendEntity() : m_newchar_optional(0)
{
	m_unkn1=0;
	m_opcode=9;//after this packet it expects opcode 5
	arr_size=0;
}
void pktCS_SendEntity::serializefrom( BitStream &src )
{
	m_cookie = src.GetPackedBits(1);
	//m_city_of_developers = src.GetBits(1);
	m_new_character=src.GetBits(1);
	if(m_new_character)
	{
		m_newchar_optional=src;
	}
}
void pktCS_SendEntity::get_new_character(PlayerEntity *ent)
{
	ACE_ASSERT(m_new_character);
	m_ent = ent;
	ent->serializefrom_newchar(m_newchar_optional);
	arr_size = m_newchar_optional.GetReadableBits()>>3;
	if(arr_size>512)
		arr_size = 512;
	m_newchar_optional.GetBitArray((u8*)arr,arr_size<<3); // unused junk, really
}

void pktCS_SendEntity::serializeto( BitStream &tgt ) const
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

void pktCS_SendEntity::DumpEntity()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_unkn1 0x%08x\n"),m_unkn1));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_b %d\n"),m_b));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    //------------Build Info-------\n")));
	if(m_ent)
		m_ent->m_char.DumpBuildInfo();
	if(m_ent)
		m_ent->dump();
}

void pktCS_SendEntity::dependent_dump( void )
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

bool pktCS_SendEntity::IsCostumePartName_NotSet( string &str )
{
	return ((str.length()==0) || (ACE_OS::strcasecmp(str.c_str(),"None")==0));
}

void pktMap_Server_EntitiesResp::sendControlState(BitStream &bs) const
{
	sendServerPhysicsPositions(bs);
	sendServerControlState(bs);
}
void pktMap_Server_EntitiesResp::sendServerPhysicsPositions(BitStream &bs) const
{
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

