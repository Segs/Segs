/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Client.cpp 253 2006-08-31 22:00:14Z malign $
 */
#include "GamePacket.h"
#include "Character.h"
void pktCS_ServerUpdate::dependent_dump()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IGame:pktCS_ServerUpdate\n%I{\n")));

	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    buildDate %08d;\n"),m_build_date));
	//		u8 clientInfo[16];
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    localMapServer %d\n"),localMapServer));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    currentVersion %s\n"),currentVersion.c_str()));
	//		u8 segsHash[16];
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    authId 0x%08x;\n"),authID));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    authCookie 0x%08x;\n"),authCookie));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    u1 %d;\n"),unkVal));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    accountName%s\n"),accountName.c_str()));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    computerData%s\n"),computerData.c_str()));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkString%s\n"),unkString.c_str()));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkString2%s\n"),unkString2.c_str()));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    tickcount 0x%08x;\n"),developerID));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    developerString %s\n"),developerString.c_str()));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

void pktCS_ServerUpdate::serializeto( BitStream &tgt ) const
{
	tgt.StorePackedBits(1, m_build_date);
	tgt.StoreBitArray(clientInfo,0x80);
	tgt.StorePackedBits(1, localMapServer);
	tgt.StoreString(currentVersion);
	tgt.StoreBitArray(segsHash,0x80);
	tgt.StorePackedBits(1, authID);
	tgt.StoreBits(32, authCookie);
	tgt.StoreString(accountName);
	tgt.StoreBits(1, unkVal);
	tgt.StoreString(unkString);
	tgt.StoreString(unkString2);

	tgt.CompressAndStoreString(computerData.c_str());
	tgt.StorePackedBits(1, developerID);
	if(developerID)
		tgt.StoreString(developerString.c_str());
}

void pktCS_ServerUpdate::serializefrom( BitStream &src )
{
/*
	m_build_date = src.GetPackedBits(1);
	src.GetBitArray(clientInfo,128);
	localMapServer = src.GetPackedBits(1);
	src.GetString(currentVersion);
	src.GetBitArray(segsHash,128);
	authID = src.GetPackedBits(1);
	authCookie = src.GetBits(32);
	src.GetString(accountName);
	unkVal = src.GetBits(1);
	src.GetString(unkString);
	src.GetString(unkString2);
	src.GetAndDecompressString(computerData);
	developerID = src.GetPackedBits(1);
	if(developerID)
		src.GetString(developerString);
*/
	m_build_date = src.GetPackedBits(1);
	u32 t = src.GetPackedBits(1);
	src.GetString(currentVersion);
	src.GetBitArray(clientInfo,128);
	authID = src.GetPackedBits(1);
	authCookie = src.GetBits(32);
	src.GetString(accountName);
}

void pktSC_MapServerAddr::dependent_dump( void )
{
	ACE_TCHAR buf[128];
	m_address.addr_to_string(buf,128);
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IGame:pktSC_MapServerAddr\n%I{\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Connect To: %s\n"),buf));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    cookie//error 0x%08x\n"),m_map_cookie));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unused1 0x%08x\n"),unused1));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unused2 0x%08x\n"),unused2));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unused3 0x%08x\n"),unused3));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unused4 0x%08x\n"),unused4));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

void pktSC_MapServerAddr::serializefrom( BitStream &src )
{
	u16 port;
	u32 ipaddr;
	unused1			= src.GetPackedBits(1);
	unused2			= src.GetPackedBits(1);
	ipaddr			= src.GetPackedBits(1);
	unused3			= src.GetPackedBits(1);
	port			= src.GetPackedBits(1);
	unused4			= src.GetPackedBits(1);
	m_map_cookie	= src.GetPackedBits(1);
	m_address.set(port,ntohl(ipaddr));
}

void pktSC_MapServerAddr::serializeto( BitStream &tgt ) const
{
	u16 port;
	u32 ipaddr;
	ipaddr			= htonl(m_address.get_ip_address());
	port			= m_address.get_port_number();
	tgt.StorePackedBits(1,unused1);
	tgt.StorePackedBits(1,unused2);
	tgt.StorePackedBits(1,ipaddr);
	tgt.StorePackedBits(1,unused3);
	tgt.StorePackedBits(1,port);
	tgt.StorePackedBits(1,unused4);
	tgt.StorePackedBits(1,m_map_cookie);
}

void pktCS_MapServerAddr_Query::dependent_dump( void )
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IGame:pktCS_MapServerAddr_Query\n%I{\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_character_index 0x%08x\n"),m_character_index));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_map_server_ip 0x%08x\n"),m_map_server_ip));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_map_number 0x%08x\n"),m_mapnumber));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_unkn2 0x%08x\n"),m_charVillain));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_char_name %s\n"),m_char_name.c_str()));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

void pktCS_MapServerAddr_Query::serializefrom( BitStream &src )
{
	m_character_index = src.GetPackedBits(1);
	m_map_server_ip = src.GetPackedBits(1);
	m_mapnumber = src.GetPackedBits(1);
	//m_charVillain = src.GetPackedBits(1);
	//m_unkn3 = src.GetBits(1);
	src.GetString(m_char_name);
}

void pktCS_MapServerAddr_Query::serializeto( BitStream &tgt ) const
{
	tgt.StorePackedBits(1,m_character_index);
	tgt.StorePackedBits(1,m_map_server_ip);
	tgt.StorePackedBits(1,m_mapnumber);
	tgt.StorePackedBits(1,m_charVillain);
	tgt.StoreBits(1,m_unkn3);
	tgt.StoreString(m_char_name);
}

void pktSC_CharSlots::dependent_dump( void )
{
	Character *act;
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IGame:pktSC_CharSlots\n%I{\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_num_slots 0x%08x\n"),m_num_slots));
	for(size_t i=0; i<m_characters.size(); i++)
	{
		act=m_characters[i];
		if(act->isEmpty())
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    character slot%d : EMPTY\n"),i));
			continue;
		}
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    character slot%d\n"),i));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    level 0x%08x\n"),act->getLevel()));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    name %s\n"),act->getName().c_str()));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    archetype %s\n"),act->getArchetype().c_str()));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    origin %s\n"),act->getOrigin().c_str()));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    bodyType 0x%08x\n"),act->getBodyType()));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn1 %f\n"),act->m_unkn1));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn2 %f\n"),act->m_unkn2));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    mapName %s\n"),act->getMapName().c_str()));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn3 0x%08x\n"),act->m_unkn3));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    unkn4 0x%08x\n"),act->m_unkn4));
	}
	for(size_t i=0; i<16; i++)
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_clientinfo[%d]:0x%08x;\n"),i,m_clientinfo[i]));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

void pktSC_CharSlots::serializefrom( BitStream &src )
{

	m_characters.clear();
	m_unknown_new =src.GetBits_2_10_24_32();
	m_authreservations=src.GetBits_2_10_24_32();
	m_last_played_idx = src.GetPackedBits(1);
	u8 num_slots = src.GetPackedBits(1);
	for(size_t i=0; i<num_slots; i++)
	{
		Character *n= new Character;
		n->serializefrom(src);
		m_characters.push_back(n);
		n->setIndex((u8)i);
	}
	ACE_ASSERT(m_characters.size()>0);
	src.GetBitArray(m_clientinfo,128);
}

void pktSC_CharSlots::serializeto( BitStream &tgt ) const
{
/*
	tgt.StoreBits_4_10_24_32(m_unknown_new);
	tgt.StoreBits_4_10_24_32(m_authreservations);
	tgt.StorePackedBits(1,m_last_played_idx);
*/
	tgt.StorePackedBits(1,static_cast<u32>(m_characters.size()));
	ACE_ASSERT(m_characters.size()>0);
	for(size_t i=0; i<m_characters.size(); i++)
	{
		m_characters[i]->serializeto(tgt);
	}
	tgt.StoreBitArray(m_clientinfo,128);
}

pktSC_CharSlots::~pktSC_CharSlots()
{
}

void pktSC_CharSlots::addCharacter(Character *c )
{
	m_characters.push_back(c);
}
void pktSC_Character::dependent_dump( void )
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IGame:pktSC_CharSlots\n%I{\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_slot_idx 0x%08x\n"),m_slot_idx));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

void pktSC_Character::serializefrom( BitStream &src )
{
	m_slot_idx = src.GetPackedBits(1);
}

void pktSC_Character::serializeto( BitStream &tgt ) const
{
	tgt.StorePackedBits(1,m_slot_idx);
	if(m_costume)
		m_costume->serializeto(tgt);
	else
		tgt.StorePackedBits(1,0); // 0 parts

}

void pktSC_EnterGameError::dependent_dump( void )
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IGame:pktSC_EnterGameError\n%I{\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_error %s\n"),m_error.c_str()));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

void pktSC_EnterGameError::serializefrom( BitStream &src )
{
	src.GetString(m_error);
}

void pktSC_EnterGameError::serializeto( BitStream &tgt ) const
{
	tgt.StoreString(m_error);
}

GamePacket * GamePacketFactory::PacketFromStream( u8 opcode,BitStream &bs ) const
{
	GamePacket *res = NULL;
	switch(opcode)
	{
	case COMM_CONTROLCOMMAND:
		res = ControlPacketFactory::PacketFromStream(opcode,bs); break;
	case COMM_CONNECT:
		res = new pktCS_Connect; break;
	case 3:
		res = new pktCS_MapServerAddr_Query; break;
	case 4:
		res = new pktCS_CharDelete; break;
	case 5:
		res = new pktCS_CharUpdate; break;
	case SERVER_UPDATE:
		res = new pktCS_ServerUpdate; break;
	default:
		res = new UnknownGamePacket;
		res->m_opcode=opcode;
		res->serializefrom(bs);
		break;
	}
	if(res)
	{
		res->m_opcode=opcode;
	}
	return res;
}

void pktCS_CharDelete::dependent_dump( void )
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IGame:pktCS_CharDelete\n%I{\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_index 0x%08x\n"),m_index));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_char_name %s\n"),m_char_name.c_str()));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

void pktCS_CharDelete::serializefrom( BitStream &src )
{
	m_index = src.GetPackedBits(1);
	src.GetString(m_char_name);
}

void pktCS_CharDelete::serializeto( BitStream &tgt ) const
{
	tgt.StorePackedBits(1,m_index);
	tgt.StoreString(m_char_name);
}

void pktSC_CharDeleteResp::serializefrom( BitStream &src )
{
	m_character_index = src.GetPackedBits(1);
}

void pktSC_CharDeleteResp::serializeto( BitStream &tgt ) const
{
	tgt.StorePackedBits(1,m_character_index);
}

void pktCS_CharUpdate::dependent_dump( void )
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IGame:pktCS_CharUpdate\n%I{\n")));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_index 0x%08x\n"),m_index));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
}

void pktCS_CharUpdate::serializefrom( BitStream &src )
{
	m_index = src.GetPackedBits(1);
}

void pktCS_CharUpdate::serializeto( BitStream &tgt ) const
{
	tgt.StorePackedBits(1,m_index);
}