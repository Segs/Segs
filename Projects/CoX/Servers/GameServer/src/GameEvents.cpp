/*
* Super Entity Game Server Project
* http://segs.sf.net/
* Copyright (c) 2009 Super Entity Game Server Team (see Authors.txt)
* This software is licensed! (See License.txt for details)
*
* $Id:$
*/
#include <ace/OS.h>
#include "HashStorage.h"
#include "GameEvents.h"
#include "CharacterClient.h"
#include "Character.h"
#include "Costume.h"
// SpecHash<std::string,val>
// 
// get hash index of given key
u32 get_hash_idx(const std::string &key)
{
	return 0;
}

void UpdateServer::dependent_dump()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IGame:pktCS_ServerUpdate\n%I{\n")));

        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    buildDate %08d;\n"),m_build_date));
        //		u8 clientInfo[16];
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    localMapServer %d\n"),localMapServer));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    currentVersion %s\n"),currentVersion.c_str()));
        //		u8 segsHash[16];
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    authId 0x%08x;\n"),authID));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    authCookie 0x%08x;\n"),authCookie));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    accountName%s\n"),accountName.c_str()));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

void UpdateServer::serializeto( BitStream &tgt ) const
{
    tgt.StorePackedBits(1, 2); //opcode
    tgt.StorePackedBits(1, m_build_date);
    tgt.StorePackedBits(1, 0); // flags
    tgt.StoreString(currentVersion);
    tgt.StoreBitArray(clientInfo,0x80);
    tgt.StorePackedBits(1, authID);
    tgt.StoreBits(32, authCookie);
    tgt.StoreString(accountName);
}

void UpdateServer::serializefrom( BitStream &src )
{
    m_build_date = src.GetPackedBits(1);
    u32 t = src.GetPackedBits(1);
    src.GetString(currentVersion);
    src.GetBitArray(clientInfo,128);
    authID = src.GetPackedBits(1);
    authCookie = src.GetBits(32);
    src.GetString(accountName);
}

void GameEntryError::serializeto( BitStream &tgt ) const
{
    tgt.StorePackedBits(1, 3); //opcode
    tgt.StoreString(m_error);
}
void GameEntryError::serializefrom( BitStream &tgt )
{
    tgt.GetString(m_error);
}

void CharacterSlots::serializeto( BitStream &tgt ) const
{
    tgt.StorePackedBits(1, 2); //opcode
    tgt.StorePackedBits(1,static_cast<u32>(m_client->max_slots()));
    ACE_ASSERT(m_client->max_slots()>0);
    for(size_t i=0; i<m_client->max_slots(); i++)
    {
        m_client->getCharacter(i)->serializetoCharsel(tgt);
    }
    tgt.StoreBitArray(m_clientinfo,128);
}

void CharacterSlots::serializefrom( BitStream &src )
{
    
}

void UpdateCharacter::serializeto( BitStream &bs ) const
{
    bs.StorePackedBits(1,5); // opcode
    bs.StorePackedBits(1,m_index);
}

void UpdateCharacter::serializefrom( BitStream &bs )
{
    m_index = bs.GetPackedBits(1);
}

void CharacterResponse::serializeto( BitStream &bs ) const
{
    ACE_ASSERT(m_client->getCharacter(m_index));
    CharacterCostume *c=0;
    if(m_client->getCharacter(m_index)->getName().compare("EMPTY")!=0) // actual character was read from db
        c=static_cast<CharacterCostume *>(m_client->getCharacter(m_index)->getCurrentCostume());
    bs.StorePackedBits(1,6); // opcode 
    if(c)
        bs.StorePackedBits(1,m_index);
    else
        bs.StorePackedBits(1,-1);
    if(c)
        c->storeCharselParts(bs);
    else
        bs.StorePackedBits(1,0); // 0 parts

}

void CharacterResponse::serializefrom( BitStream &bs )
{
    ACE_ASSERT(!"TODO");
}