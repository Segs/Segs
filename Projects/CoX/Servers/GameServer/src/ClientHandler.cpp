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
	case 0: // control codes 
		{
			ControlPacket * ctrl_pak = static_cast<ControlPacket *>(pak);//CTRL_DISCONNECT_REQ
			if(ctrl_pak->m_comm_opcode==CTRL_DISCONNECT_REQ)
			{
				if(m_client)
				{
					if(m_client)
						m_server->disconnectClient(m_client);
					m_client=0;
					m_proto->SendPacket(new pktSC_Diconnect);
				}
			}

		}
	case CMSG_SERVER_UPDATE:
		{
			return onServerUpdate(pak);
		}
	case CMSG_ENTER_MAP_QUERY:
		{
			onOnterMap(pak);
		}
	case CMSG_CHARACTER_DELETE:
		{
			onCharacterDelete(pak);
		}
	case CMSG_CHARACTER_UPDATE:
		{	
			onCharacterUpdate(pak);
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
	pktSC_Connected *res = new pktSC_Connected;
	m_proto->SendPacket(res);
	return true;
}

bool CharacterHandler::sendNotAuthorized()
{
	pktSC_EnterGameError *res = new pktSC_EnterGameError();
	res->m_error = "Unauthorized access attempt !";
	m_proto->SendPacket(res);
	if(m_client)
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
	if(m_client)
		m_server->disconnectClient(m_client);
	return true;
}

CharacterDatabase * CharacterHandler::getDb()
{
	return m_server->getDb();
}

bool CharacterHandler::onServerUpdate( GamePacket * pak )
{
	pktCS_ServerUpdate * su_pak = (pktCS_ServerUpdate * )pak;
	pak->dump();
	m_client = m_server->ClientExpected(getTargetAddr(),su_pak);
	if(su_pak->m_build_date!=supported_version)
		return sendWrongVersion(su_pak->m_build_date);

	if(!m_client)
		return sendNotAuthorized();

	m_client->setHandler(this);

	if(!m_client->serializeFromDb())
		return sendNotAuthorized(); //TODO: send db error here

	m_client->getCharsFromDb();
	m_proto->SendPacket(new pktSC_CharSlots(m_client));
	break;
}

void CharacterHandler::onOnterMap( GamePacket * pak )
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
		map_iface->AssociatePlayerWithMap(m_client->getId(),query->m_char_name,query->m_mapnumber);
		m_proto->SendPacket(res);
		break;
	}
	pktSC_EnterGameError *entry_error = new pktSC_EnterGameError();
	entry_error->m_error = "There are no available Map Servers.";
	m_proto->SendPacket(entry_error);
	break;
}

void CharacterHandler::onCharacterDelete( GamePacket * pak )
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

void CharacterHandler::onCharacterUpdate( GamePacket * pak )
{
	pktCS_CharUpdate *up = static_cast<pktCS_CharUpdate *>(pak);
	ACE_DEBUG ((LM_WARNING,ACE_TEXT ("Char update req! %d;\n"),up->m_index));
	ACE_ASSERT(m_client->getMaxSlots()>up->m_index);

	pktSC_Character *res = new pktSC_Character;
	res->m_slot_idx	= up->m_index;
	res->m_costume	= m_client->getCharacter(up->m_index)->getCurrentCostume();
	m_proto->SendPacket(res); 
	break;
}