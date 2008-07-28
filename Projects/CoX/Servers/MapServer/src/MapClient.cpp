#include <stdarg.h>
#include "Client.h"
#include "MapPacket.h"
#include "MapHandler.h"
#include "Entity.h"
MapClient::MapClient() : m_ent(NULL)
{

}

void MapClient::AddShortcut(int index, NetCommand *command)
{
	if(m_shortcuts.find(index)!=m_shortcuts.end())
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("Replacing command %d %s->%s\n"),index,m_shortcuts[index]->m_name.c_str(),command->m_name.c_str()));
		
	}
	m_shortcuts[index]=command;
}
void MapClient::SendCommand(NetCommand *command,...)
{
	ACE_ASSERT(!"Not implemented yet");
}
// This packet tells us to fill in client data from db/create a new client
GamePacket * MapClient::HandleClientPacket( pktCS_SendEntity *ent )
{
	GameServerInterface *giface = ServerManager::instance()->GetGameServer(0);
	//giface->getCharacterData(getId());
	// Client sends us it's entity 
	//	At this point we either got a new character, or an old one.
	//		Ask game server to provide us with character data object.
	//Entity * character_ent = m_server->getGameInterface()->getCharacterData(m_client->getId());
	//		If this fails: call m_i_gameserver->NewCharacter(client->id);
	//if(!character_ent)
	//{
	//	character_ent = in_pak->m_ent;
	//	m_server->m_i_game->newCharacter(m_client->getId(),character_ent);
	//}
	//	Embed character data in m_client object.
	//		m_client->setCharEntity(character_ent);
	//character_ent->
	//	Create initial routing for all relevant packets to appropriate Map thread.
	//	

	pktSC_Connect *res = new pktSC_Connect;
	res->m_resp=1;
	res->m_unkn1=-1.0;
	return res;

}
