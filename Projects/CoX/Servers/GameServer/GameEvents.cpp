/*
* Super Entity Game Server Project
* http://segs.sf.net/
* Copyright (c) 2009 Super Entity Game Server Team (see Authors.txt)
* This software is licensed! (See License.txt for details)
*

*/
#include <ace/Log_Msg.h>
#include "HashStorage.h"
#include "GameEvents.h"
#include "CharacterClient.h"
#include "Character.h"
#include "Costume.h"
// SpecHash<std::string,val>
//
// get hash index of given key
//uint32_t get_hash_idx(const std::string &)
//{
//    return 0;
//}

void UpdateServer::dependent_dump()
{
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IGame:pktCS_ServerUpdate\n%I{\n")));

        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    buildDate %08d;\n"),m_build_date));
        //uint8_t clientInfo[16];
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    localMapServer %d\n"),localMapServer));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    currentVersion %s\n"),qPrintable(currentVersion)));
        //uint8_t segsHash[16];
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    authId 0x%08x;\n"),authID));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    authCookie 0x%08x;\n"),authCookie));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    accountName%s\n"),qPrintable(accountName)));
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

void UpdateServer::serializeto( BitStream &tgt ) const
{
    tgt.StorePackedBits(1, 2); //opcode
    tgt.StorePackedBits(1, m_build_date);
    tgt.StorePackedBits(1, 0); // flags
    tgt.StoreString(currentVersion);
    tgt.StoreBitArray(clientInfo,sizeof(clientInfo)*8);
    tgt.StorePackedBits(1, authID);
    tgt.StoreBits(32, authCookie);
    tgt.StoreString(accountName);
}

void UpdateServer::serializefrom( BitStream &src )
{
    m_build_date = src.GetPackedBits(1);
    /*uint32_t t =*/ src.GetPackedBits(1);
    src.GetString(currentVersion);
    src.GetBitArray(clientInfo,sizeof(clientInfo)*8);
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
    tgt.StorePackedBits(1,static_cast<uint32_t>(m_client->max_slots()));
    assert(m_client->max_slots()>0);
    for(size_t i=0; i<m_client->max_slots(); i++)
    {
        m_client->getCharacter(i)->serializetoCharsel(tgt);
    }
    //tgt.StoreBitArray(m_clientinfo,128);
}

void CharacterSlots::serializefrom( BitStream &)
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
    Character *indexed_character = m_client->getCharacter(m_index);
    CharacterCostume *c=0;
    assert(indexed_character);
    if(indexed_character->getName().compare("EMPTY")!=0) // actual character was read from db
        c=static_cast<CharacterCostume *>(indexed_character->getCurrentCostume());
    bs.StorePackedBits(1,6); // opcode

    if(c)
    {
        bs.StorePackedBits(1,m_index);
        c->storeCharselParts(bs);
    }
    else
    {
        bs.StorePackedBits(1,-1);
        bs.StorePackedBits(1,0); // 0 parts
    }
}

void CharacterResponse::serializefrom( BitStream &bs )
{
    bs.GetPackedBits(1);
    assert(!"TODO");
}

void DeletionAcknowledged::serializeto( BitStream &tgt ) const
{
    tgt.StorePackedBits(1,5); // opcode 5
}
