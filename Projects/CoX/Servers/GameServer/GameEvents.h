/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#pragma once
#include "LinkLevelEvent.h"
#include "BitStream.h"
#include "GameLink.h"
#include "CRUDP_Protocol/CRUD_Link.h"
#include "CRUDP_Protocol/CRUD_Events.h"
#include "Servers/InternalEvents.h"

#include <QtCore/QString>

typedef CRUDLink_Event GameLinkEvent;

struct GameAccountData;

class GameEventTypes : public CRUD_EventTypes
{
public:
        BEGINE_EVENTS(CRUD_EventTypes)
        EVENT_DECL(evUpdateServer,      0)
        EVENT_DECL(evMapAddrRequest,    1)
        EVENT_DECL(evDeleteCharacter,   2)
        EVENT_DECL(evUpdateCharacter,   3)
        EVENT_DECL(evGameEntryError,    4)
        EVENT_DECL(evCharacterSlots,    5)
        EVENT_DECL(evCharacterResponse, 6)
        EVENT_DECL(evMapAddrResponse,   7)
        EVENT_DECL(evDeleteAcknowledged,8)

        EVENT_DECL(evUnknownEvent,16)
        END_EVENTS(17)
};

class MapServerAddrRequest : public GameLinkEvent
{
public:
    MapServerAddrRequest() : GameLinkEvent(GameEventTypes::evMapAddrRequest)
    {

    }
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,3); //opcode
    }
    void serializefrom(BitStream &src)
    {
        m_character_index = src.GetPackedBits(1);
        m_map_server_ip = src.GetPackedBits(1);
        m_mapnumber = src.GetPackedBits(1);
        src.GetString(m_char_name);

    }
    uint32_t m_map_server_ip;
    uint16_t m_character_index;
    uint32_t m_mapnumber;
    QString m_char_name;
};
class MapServerAddrResponse : public GameLinkEvent
{
public:
    MapServerAddrResponse() : GameLinkEvent(GameEventTypes::evMapAddrResponse)
    {
        unused1=unused2=unused3=unused4=0;
    }
    void serializefrom(BitStream &src)
    {
        uint16_t port;
        uint32_t ipaddr;
        unused1         = src.GetPackedBits(1);
        unused2         = src.GetPackedBits(1);
        ipaddr          = src.GetPackedBits(1);
        unused3         = src.GetPackedBits(1);
        port            = src.GetPackedBits(1);
        unused4         = src.GetPackedBits(1);
        m_map_cookie    = src.GetPackedBits(1);
        m_address.set(port,ntohl(ipaddr));
    }
    void serializeto(BitStream &tgt) const
    {
        uint32_t ipaddr = htonl(m_address.get_ip_address());
        uint16_t port   = m_address.get_port_number();
        tgt.StorePackedBits(1,4); //opcode
        tgt.StorePackedBits(1,unused1);
        tgt.StorePackedBits(1,unused2);
        tgt.StorePackedBits(1,ipaddr);
        tgt.StorePackedBits(1,unused3);
        tgt.StorePackedBits(1,port);
        tgt.StorePackedBits(1,unused4);
        tgt.StorePackedBits(1,m_map_cookie); // this should be 0 if there was an error ( like "InvalidName" )
    }
    uint8_t unused1;
    uint8_t unused2;
    uint8_t unused3;
    uint8_t unused4;
    ACE_INET_Addr m_address;
    // 0 - Name already taken.
    // 1 - Problem detected in the game database system
    uint32_t m_map_cookie;
};
class DeleteCharacter : public GameLinkEvent
{
public:
    DeleteCharacter():GameLinkEvent(GameEventTypes::evDeleteCharacter)
    {}
    DeleteCharacter(EventProcessor *evsrc,uint8_t idx,const QString &name) : GameLinkEvent(GameEventTypes::evDeleteCharacter,evsrc),m_index(idx),m_char_name(name)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,4); // opcode
        bs.StorePackedBits(1,m_index);
        bs.StoreString(m_char_name);
    }
    void serializefrom(BitStream &bs)
    {
        m_index=bs.GetPackedBits(1);
        bs.GetString(m_char_name);
    }
    uint8_t m_index;
    QString m_char_name;
};
class UpdateCharacter : public GameLinkEvent
{
public:
    UpdateCharacter():GameLinkEvent(GameEventTypes::evUpdateCharacter)
    {
    }
    void serializeto(BitStream &bs) const;
    void serializefrom(BitStream &bs);
    uint8_t m_index;
};
class CharacterResponse : public GameLinkEvent
{
public:
    CharacterResponse():GameLinkEvent(GameEventTypes::evCharacterResponse)
    {}
    CharacterResponse(EventProcessor *src,uint8_t idx,GameAccountData *gad) : GameLinkEvent(GameEventTypes::evCharacterResponse,src)
    {
        m_index=idx;
        m_data=gad;
    }
    void set_client(GameAccountData *gad) {m_data=gad;}
    void serializeto(BitStream &bs) const;
    void serializefrom(BitStream &bs);
    uint8_t m_index;
    GameAccountData *m_data;
};

class UpdateServer : public GameLinkEvent
{
public:
    UpdateServer():GameLinkEvent(GameEventTypes::evUpdateServer)
    {}
    void serializeto( BitStream &tgt ) const;
    void serializefrom( BitStream &src );
    void dependent_dump();


    uint32_t m_build_date;
    QString currentVersion;
    uint8_t clientInfo[16];
    uint32_t authID, authCookie;
    QString accountName;
    bool localMapServer;
};
class CharacterSlots : public GameLinkEvent
{
public:
    CharacterSlots():GameLinkEvent(GameEventTypes::evCharacterSlots)
    {}
    void set_account_data(GameAccountData *c) {m_data=c;}
    void serializeto( BitStream &tgt ) const;
    void serializefrom( BitStream &src );
    void dependent_dump();

    uint32_t m_unknown_new;
    uint8_t m_clientinfo[16];
    GameAccountData *m_data;
};

class GameEntryError : public GameLinkEvent
{
public:
    GameEntryError():GameLinkEvent(GameEventTypes::evGameEntryError)
    {}
    GameEntryError(EventProcessor *evsrc,const QString &erstr):GameLinkEvent(GameEventTypes::evGameEntryError,evsrc),m_error(erstr)
    {}
    void serializeto( BitStream &tgt ) const;
    void serializefrom( BitStream &src );
    void dependent_dump();
    QString m_error;
};
class DeletionAcknowledged : public GameLinkEvent
{
public:
    DeletionAcknowledged():GameLinkEvent(GameEventTypes::evDeleteAcknowledged)
    {}
    void serializeto( BitStream &tgt ) const;
    void serializefrom( BitStream &) {}
};
class GameUnknownRequest : public GameLinkEvent
{
public:
    GameUnknownRequest():GameLinkEvent(GameEventTypes::evUnknownEvent)
    { }
    void serializeto(BitStream &) const
    { }
    void serializefrom(BitStream &)
    { }
};
