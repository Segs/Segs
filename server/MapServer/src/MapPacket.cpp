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