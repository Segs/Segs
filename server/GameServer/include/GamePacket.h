/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: GamePacket.h 310 2007-01-25 10:51:06Z nemerle $
 */

#pragma once
#include "PacketBase.h"
#include "CommonNetStructures.h"
#include <ace/os_include/os_string.h>
#include <ace/OS_NS_strings.h>
#include <ace/OS.h>
class Character;
class CharacterCostume;
// 20060404 -> AuthReservationsRemaining etc.
/*
	key: 
		pakCS* client -> server packets
		pakSC  server -> client packets
*/
class pktCS_ServerUpdate : public GamePacket
{
protected:
	virtual void dependent_dump();

public:

	virtual void serializeto(BitStream &tgt) const;
	virtual void serializefrom(BitStream &src);

	u32 m_build_date;
	u8 clientInfo[16];
	bool localMapServer;
	string currentVersion;
	u8 segsHash[16];
	u32 authID, authCookie;
	bool unkVal;			//	This can be either true or false, it doesn't seem to effect much
	string accountName;
	string computerData;
	string unkString;		//	This string is blank in my client - should be investigated further
	string unkString2;		//	This string is blank in my client - should be investigated further
	u32 developerID;		//	Unsure of this one, should be set to zero, unless haxing =p
	string developerString;	//	Don't know what actually goes here yet, but i'll find out

};

class pktSC_MapServerAddr : public GamePacket
{
protected:
	virtual void dependent_dump(void);
public:
	pktSC_MapServerAddr()
	{
		m_opcode=4;
        unused1=unused2=unused3=unused4=0;
	}
	virtual void serializefrom(BitStream &src);
	virtual void serializeto(BitStream &tgt) const;
	u8 unused1;
	u8 unused2;
	u8 unused3;
	u8 unused4;
	ACE_INET_Addr m_address;
	// 0 - Name already taken.
	// 1 - Problem detected in the game database system
	u32 m_map_cookie;

};

class pktCS_MapServerAddr_Query : public GamePacket
{
protected:
	virtual void dependent_dump(void);
public:
	pktCS_MapServerAddr_Query()
	{
		m_opcode=3;
	}
	virtual void serializefrom(BitStream &src);
	virtual void serializeto(BitStream &tgt) const;
	u32 m_map_server_ip;
	u32 m_character_index;
	u32 m_mapnumber;
	u32 m_charVillain;
	std::string m_char_name;
	bool m_unkn3;
};
class pktCS_CharDelete : public GamePacket
{
protected:
	virtual void dependent_dump(void);
public:
	pktCS_CharDelete()
	{
		m_opcode=4;
	}
	virtual void serializefrom(BitStream &src);
	virtual void serializeto(BitStream &tgt) const;
	u8 m_index;
	string m_char_name;
};
class pktCS_CharUpdate : public GamePacket
{
protected:
	virtual void dependent_dump(void);
public:
	pktCS_CharUpdate()
	{
		m_opcode=5;
	}
	virtual void serializefrom(BitStream &src);
	virtual void serializeto(BitStream &tgt) const;
	u8 m_index;
};

class pktSC_CharSlots : public GamePacket
{
protected:
	virtual void dependent_dump(void);
	u32 m_authreservations;
	u8 m_last_played_idx;
public:
	pktSC_CharSlots()
	{
		m_opcode=2;
		m_num_slots=0;
		m_last_played_idx=0;
		m_authreservations=1;
		m_unknown_new=0;
	}
	~pktSC_CharSlots();
	virtual void serializefrom(BitStream &src);
	virtual void serializeto(BitStream &tgt) const;

	void addCharacter(Character *c); //! this should be called at least once
	u32 m_unknown_new;
	u8 getLast_played_idx() const { return m_last_played_idx; }
	void setLast_played_idx(u8 val) { m_last_played_idx = val; }
	u32 getAuthreservations() const { return m_authreservations; }
	void setAuthreservations(u32 val) { m_authreservations = val; }
	u8 m_num_slots;
	vector<Character *> m_characters;
	u8 m_clientinfo[16];
};
// The contents are parsed in the client for some unknown purposes !
class pktSC_EnterGameError : public GamePacket
{
protected:
	virtual void dependent_dump(void);
public:
	pktSC_EnterGameError()
	{
		m_opcode=3;
	}
	virtual void serializefrom(BitStream &src);
	virtual void serializeto(BitStream &tgt) const;
	std::string m_error;
};

class pktSC_Character : public GamePacket
{
protected:
	virtual void dependent_dump(void);
public:
	static pktSC_Character *null_character_packet;
	pktSC_Character()
	{
		m_opcode=6;
	}
	virtual void serializefrom(BitStream &src);
	virtual void serializeto(BitStream &tgt) const;
	u8 m_slot_idx;
	CharacterCostume *m_costume;
};
class pktSC_CharDeleteResp : public GamePacket
{
protected:
	virtual void dependent_dump(void)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%IpktSC_CharDeleteResp\n%I{\n")));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    m_character_index 0x%08x\n"),m_character_index));
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I}\n")));
	}
public:
	pktSC_CharDeleteResp()
	{
		m_opcode=5;
	}
	virtual void serializefrom(BitStream &src);
	virtual void serializeto(BitStream &tgt) const;
	u32 m_character_index;
};


class GamePacketFactory : public ControlPacketFactory
{
public:
	virtual GamePacket *PacketFromStream(u8 opcode,BitStream &bs) const;

	virtual GamePacket *PacketFromStream(BitStream &bs)
	{
		eGameOpcode opcode = (eGameOpcode)bs.GetPackedBits(1);
		return PacketFromStream(opcode,bs);
	}
};


