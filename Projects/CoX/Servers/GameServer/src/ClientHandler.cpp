/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: ClientHandler.cpp 319 2007-01-26 17:03:18Z nemerle $
 */
#include <sstream>
#include "ClientHandler.h"
#include "GamePacket.h"
#include "GameProtocol.h"
#include "GameServer.h"
#include "ServerManager.h"
#include "MapServerInterface.h"
#include "CharacterClient.h"
#include "Character.h"
#include "opcodes/ControlCodes.h"
#include "opcodes/Opcodes.h"

static const u32 supported_version=20040422;
typedef struct
{
	u8 unk_1[12];
	u32 unk_2;
} client_info;
CharacterHandler::CharacterHandler(GameServer *srv):m_server(srv)
{
	m_client=0;
	
}
CharacterHandler::~CharacterHandler()
{
	m_server=0;
	m_client=0;
}
bool CharacterHandler::ReceivePacket(GamePacket *pak)
{
/*
	The following part doesn't need anything from the client.
*/
	ACE_ASSERT(m_server);
	if(ReceiveControlPacket((ControlPacket *)pak))
	{
		return true;
	}
	if(pak->getType()==COMM_CONNECT)
	{
		return sendAllowConnection();
	}
	pak->dump();
	switch(pak->getType())
	{
	case 2:
		{
			//Character *act;
			pktCS_ServerUpdate * su_pak = (pktCS_ServerUpdate * )pak;
			pktSC_Character * char_pak;
			pak->dump();
			if(su_pak->m_build_date!=supported_version)
			{
				return sendWrongVersion(su_pak->m_build_date);
			}
			m_client = m_server->ClientExpected(getTargetAddr(),su_pak);
			if(!m_client)
			{
				return sendNotAuthorized();
			}
			m_client->setHandler(this);
			if(!m_client->serializeFromDb())
			{
				return sendNotAuthorized(); //TODO: send db error here
			}
			m_client->getCharsFromDb();
			pktSC_CharSlots *res = new pktSC_CharSlots;
			vector<pktSC_Character *> characters_to_send;
			res->setLast_played_idx(0); //m_client->getLastPlayedCharSlot();

			for(size_t i=0; i<m_client->getMaxSlots(); i++)
			{
				res->addCharacter(m_client->getCharacter(i));
				if(res->m_characters[i]->isEmpty())
					continue;
				char_pak = new pktSC_Character;
				char_pak->m_slot_idx=i;
				char_pak->m_costume=res->m_characters[i]->m_costume;
				characters_to_send.push_back(char_pak);
			}
			srand((u32)time(NULL));
			res->m_unknown_new=rand()&0xFF;
			res->setAuthreservations(4); // slots 0-3 are playable slots
			res->setLast_played_idx(4); // last playable
			for(int i=0; i<16; i++)
				res->m_clientinfo[i]=0xFF;//rand()&0xFF;
			u32 VALENTINE_enabled = 1<<16; // Event_Valentine_Accessories
			u32 CrabSpiderRED_enabled = 1<<18;
			u32 BloodWidow_enabled = 1<<19;  // +WolfspiderRed
			u32 mystichelm_enabled = 1<<20;  // +Mystic
			u32 CrabSpider_enabled = 1<<21;
			u32 segsDVD_enabled   = 1<<22;
			u32 villain_enabled  = 1<<23;		
			u32 spec_flag1		 = 4<<24; // 3 bits : 1 - EB, 2 - GameStop,3 - BestBuy, 4 - Generic
			u32 DVD_Edition		 = 1<<27;
			u32 warshade_enabled = 1<<28;
			u32 hero_enabled	 = 1<<31;

			((client_info*)&(res->m_clientinfo))->unk_2=hero_enabled|warshade_enabled|villain_enabled|CrabSpiderRED_enabled|BloodWidow_enabled|mystichelm_enabled|segsDVD_enabled|VALENTINE_enabled|spec_flag1|DVD_Edition;
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("0x%08x;\n"),((client_info*)&(res->m_clientinfo))->unk_2));
			res->dump();
			m_proto->SendPacket(res);
			for(size_t i=0; i<characters_to_send.size(); i++)
				m_proto->SendPacket(characters_to_send[i]);
			break;
		}
	case 3:
		{
			pktCS_MapServerAddr_Query *query = (pktCS_MapServerAddr_Query *)pak;
			// expects packet with op 4 in return
			pktSC_MapServerAddr *res=new pktSC_MapServerAddr;
			//res->m_error_number=0; name already taken
			res->m_map_cookie=1; // problem in database system
			if(ServerManager::instance()->MapServerCount()>0)
			{
				MapServerInterface *map_iface = ServerManager::instance()->GetMapServer(0);
				res->m_map_cookie = map_iface->ExpectClient(getTargetAddr(),m_client->getId(),m_client->getAccessLevel());//0 -> invalid name. 1 -> game databes problem detected
				res->m_address		= map_iface->getAddress();
				//AssociateCharacterWithClient();
				map_iface->AssociatePlayerWithMap(m_client->getId(),query->m_mapnumber);
				m_proto->SendPacket(res);
				break;
			}
			pktSC_EnterGameError *entry_error = new pktSC_EnterGameError();
			entry_error->m_error = "There are no available Map Servers.";
			m_proto->SendPacket(entry_error);
			break;
		}
	case 4:
		{
			pktCS_CharDelete *del= (pktCS_CharDelete *)pak;
			//m_client->deleteCharacter(del->)
			ACE_DEBUG ((LM_WARNING,ACE_TEXT ("Ye gods, a deletion attempt ! %d:%s;\n"),del->m_index,del->m_char_name.c_str()));
			pktSC_EnterGameError *res = new pktSC_EnterGameError();
			//res->m_error = "\"No\" \"Test\"<b>MapServers</b> available.";
			//res->m_error = "P1071721040";
			//res->m_error = "\"UnknownAuthCode\", \"12\"";
			//res->m_error = "\"%d\" \"PlayerLevel\"";
			m_proto->SendPacket(res);
			break;
		}
	case 5:
		{	
			pktCS_CharUpdate *up = static_cast<pktCS_CharUpdate *>(pak);
			ACE_DEBUG ((LM_WARNING,ACE_TEXT ("Char update req! %d;\n"),up->m_index));
			ACE_ASSERT(m_client->getMaxSlots()>up->m_index);
			pktSC_Character *res = new pktSC_Character;
			res->m_slot_idx=up->m_index;
			res->m_costume=m_client->getCharacter(up->m_index)->m_costume;
			m_proto->SendPacket(res); 
			break;
		}
	default:
		pak->dump();

		pktSC_EnterGameError *res = new pktSC_EnterGameError();
		//res->m_error = "UnknownAuthCode \043\000";
		res->m_error = "UnknownAuthCode"; // how does cox  take a %i argument ??
		for(int i=0; i<4;)
			res->m_error += "\016";

		//\"{num=12}\"\n";
		m_proto->SendPacket(res); 

	}
	return true;
}
void CharacterHandler::setClient(IClient *cl)
{
	m_client=static_cast<CharacterClient *>(cl);
}

bool CharacterHandler::sendAllowConnection()
{
	pktConnected *res = new pktConnected;
	m_proto->SendPacket(res);
	return true;
}

bool CharacterHandler::sendNotAuthorized()
{
	pktSC_EnterGameError *res = new pktSC_EnterGameError();
	res->m_error = "Unauthorized access attempt !";
	m_proto->SendPacket(res);
	m_server->disconnectClient(m_client);
	return true;
}
bool CharacterHandler::sendWrongVersion(u32 version)
{
	stringstream stream;
	stream << "We are sorry but client version:"<<version<<" is not supported";
	pktSC_EnterGameError *res = new pktSC_EnterGameError();
	res->m_error = stream.str();
	m_proto->SendPacket(res);
	m_server->disconnectClient(m_client);
	return true;
}

CharacterDatabase * CharacterHandler::getDb()
{
	return m_server->getDb();
}
