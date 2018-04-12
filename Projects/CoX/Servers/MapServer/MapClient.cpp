#include "MapClient.h"

#include "Character.h"
#include "Entity.h"
#include "AdminServer/AccountInfo.h"
#include "NetCommandManager.h"

#include <cstdarg>

void MapClientSession::AddShortcut(int index, NetCommand *command)
{
    if(m_shortcuts.find(index)!=m_shortcuts.end())
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("Replacing command %d %s->%s\n"),index,qPrintable(m_shortcuts[index]->m_name),qPrintable(command->m_name)));
    }
    m_shortcuts[index]=command;
}

bool MapClientSession::db_create()
{
    account_info().fill_game_db(0);
    Character * new_char = account_info().create_new_character();
    if(!new_char)
        return false;
    *new_char = *m_ent->m_char;
    new_char->setName(m_name);
    account_info().store_new_character(m_ent, new_char); // Must pass both entity and char.

    return true;
}

void MapClientSession::entity( Entity * val )
{
    m_ent = val;
    m_ent->m_char->setName(m_name); // this is used because the new characters are passed to us nameless
}
