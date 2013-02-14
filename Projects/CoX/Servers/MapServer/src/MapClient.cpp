#include <stdarg.h>
#include "MapClient.h"
#include "MapHandler.h"
#include "Entity.h"
MapClient::MapClient() : m_ent(0),m_tick_source(0)
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
    assert(!"Not implemented yet");
}

bool MapClient::db_create()
{
    account_info().fill_game_db(0);
    Character * new_char = account_info().create_new_character();
    if(!new_char)
        return false;
    *new_char = m_ent->m_char;
    new_char->setName(m_name);
    account_info().store_new_character(new_char);
    return true;
}

void MapClient::entity( Entity * val )
{
    m_ent = val;
    m_ent->m_char.setName(m_name); // this is used because the new characters are passed to us nameless
}
// This packet tells us to fill in client data from db/create a new character
/*
GamePacket * MapClient::HandleClientPacket( pktCS_SendEntity *ent )
{
    // Client sends us it's entity
    GameServerInterface *giface = ServerManager::instance()->GetGameServer(0);
    //	At this point we either got a new character, or an old one.
    if(ent->m_new_character)
    {
        assert(ent->m_ent);
        //	character_ent = in_pak->m_ent;
        //	m_server->m_i_game->newCharacter(m_client->getId(),character_ent);
    }
    else
    {
        //		Ask game server to provide us with character data object.
        //Entity * character_ent = m_server->getGameInterface()->getCharacterData(m_client->getId());

    }
    //		If this fails: call m_i_gameserver->NewCharacter(client->id);
    //if(!character_ent)
    //{
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
*/
