/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Components/LinkLevelEvent.h"
#include "Components/BitStream.h"
//#include "GameLink.h"
//#include "CRUDP_Protocol/CRUD_Link.h"
#include "Common/CRUDP_Protocol/CRUD_Events.h"
#include "Common/Servers/InternalEvents.h"

// GameEvents make use of some of the event Data defined in GameDBSyncEvents
#include "Common/Messages/GameDatabase/GameDBSyncEvents.h"
#include <QtCore/QString>
#include <array>

namespace SEGSEvents
{
using GameLinkEvent = CRUDLink_Event;

struct GameAccountResponseData;

enum GameEventTypes
{
        BEGINE_EVENTS(GameEventTypes,CRUD_EventTypes)
        evUpdateServer,
        evMapServerAddrRequest,
        evDeleteCharacter,
        evUpdateCharacter,
        evGameEntryError,
        evCharacterSlots,
        evCharacterResponse,
        evMapServerAddrResponse,
        evDeleteAcknowledged,
        evServerReconfigured = evDeleteAcknowledged+100,
        END_EVENTS(GameEventTypes,102)
};

// [[ev_def:type]]
class MapServerAddrRequest : public GameLinkEvent
{
public:
    MapServerAddrRequest() : GameLinkEvent(GameEventTypes::evMapServerAddrRequest)
    {

    }
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,3); //opcode
    }
    void serializefrom(BitStream &src) override
    {
        m_character_index = src.GetPackedBits(1);
        m_map_server_ip = src.GetPackedBits(1);
        m_mapnumber = src.GetPackedBits(1);
        src.GetString(m_char_name);

    }
    // [[ev_def:field]]
    uint32_t m_map_server_ip;
    // [[ev_def:field]]
    uint16_t m_character_index;
    // [[ev_def:field]]
    uint32_t m_mapnumber;
    // [[ev_def:field]]
    QString m_char_name;

    EVENT_IMPL(MapServerAddrRequest)
};

// [[ev_def:type]]
class MapServerAddrResponse : public GameLinkEvent
{
public:
    MapServerAddrResponse() : GameLinkEvent(GameEventTypes::evMapServerAddrResponse)
    {
        unused1=unused2=unused3=unused4=0;
    }
    void serializefrom(BitStream &src) override
    {
        unused1         = src.GetPackedBits(1);
        unused2         = src.GetPackedBits(1);
        uint32_t ipaddr = src.GetPackedBits(1);
        unused3         = src.GetPackedBits(1);
        uint16_t port   = src.GetPackedBits(1);
        unused4         = src.GetPackedBits(1);
        m_map_cookie    = src.GetPackedBits(1);
        m_address.set(port,ntohl(ipaddr));
    }
    void serializeto(BitStream &tgt) const override
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
    // [[ev_def:field]]
    uint8_t unused1;
    // [[ev_def:field]]
    uint8_t unused2;
    // [[ev_def:field]]
    uint8_t unused3;
    // [[ev_def:field]]
    uint8_t unused4;
    // [[ev_def:field]]
    ACE_INET_Addr m_address;
    /// @note m_map_cookie has two special values:
    /// 0 - Name already taken.
    /// 1 - Problem detected in the game database system
    // [[ev_def:field]]
    uint32_t m_map_cookie;
    EVENT_IMPL(MapServerAddrResponse)
};

// [[ev_def:type]]
class DeleteCharacter : public GameLinkEvent
{
public:
    DeleteCharacter():GameLinkEvent(GameEventTypes::evDeleteCharacter)
    {}
    DeleteCharacter(EventProcessor *evsrc,uint8_t idx,const QString &name) : GameLinkEvent(GameEventTypes::evDeleteCharacter,evsrc),m_index(idx),m_char_name(name)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,4); // opcode
        bs.StorePackedBits(1,m_index);
        bs.StoreString(m_char_name);
    }
    void serializefrom(BitStream &bs) override
    {
        m_index=bs.GetPackedBits(1);
        bs.GetString(m_char_name);
    }
    // [[ev_def:field]]
    uint8_t m_index;
    // [[ev_def:field]]
    QString m_char_name;
    EVENT_IMPL(DeleteCharacter)
};

// [[ev_def:type]]
class UpdateCharacter : public GameLinkEvent
{
public:
    UpdateCharacter():GameLinkEvent(GameEventTypes::evUpdateCharacter)
    {
    }
    void serializeto(BitStream &bs) const override;
    void serializefrom(BitStream &bs) override;
    // [[ev_def:field]]
    uint8_t m_index;
    EVENT_IMPL(UpdateCharacter)
};

// [[ev_def:type]]
class CharacterResponse : public GameLinkEvent
{
public:
    CharacterResponse():GameLinkEvent(GameEventTypes::evCharacterResponse)
    {}
    CharacterResponse(EventProcessor *src,uint8_t idx,const GameAccountResponseData &gad) : GameLinkEvent(GameEventTypes::evCharacterResponse,src)
    {
        m_index=idx;
        m_data=gad;
    }
    void serializeto(BitStream &bs) const override;
    void serializefrom(BitStream &bs) override;
    // [[ev_def:field]]
    uint8_t m_index;
    // [[ev_def:field]]
    GameAccountResponseData m_data;
    EVENT_IMPL(CharacterResponse)
};

// [[ev_def:type]]
class UpdateServer : public GameLinkEvent
{
public:
    UpdateServer():GameLinkEvent(GameEventTypes::evUpdateServer)
    {}
    void serializeto( BitStream &tgt ) const override;
    void serializefrom( BitStream &src ) override;
    void dependent_dump() const;
    EVENT_IMPL(UpdateServer)


    // [[ev_def:field]]
    uint32_t m_build_date;
    // [[ev_def:field]]
    QString currentVersion;
    // [[ev_def:field]]
    std::array<uint8_t,16> clientInfo;
    // [[ev_def:field]]
    uint32_t authID;
    // [[ev_def:field]]
    uint32_t authCookie;
    // [[ev_def:field]]
    QString accountName;
    // [[ev_def:field]]
    bool localMapServer;
};

// [[ev_def:type]]
class CharacterSlots : public GameLinkEvent
{
public:
    CharacterSlots():GameLinkEvent(GameEventTypes::evCharacterSlots)
    {}
    void serializeto( BitStream &tgt ) const override;
    void serializefrom( BitStream &src ) override;

    // [[ev_def:field]]
    uint32_t m_unknown_new;
    // [[ev_def:field]]
    std::array<uint8_t,16> m_clientinfo;
    // [[ev_def:field]]
    GameAccountResponseData m_data;
    EVENT_IMPL(CharacterSlots)
};

// [[ev_def:type]]
class GameEntryError : public GameLinkEvent
{
public:
    GameEntryError():GameLinkEvent(GameEventTypes::evGameEntryError)
    {}
    GameEntryError(EventProcessor *evsrc,const QString &erstr):GameLinkEvent(GameEventTypes::evGameEntryError,evsrc),m_error(erstr)
    {}
    void serializeto( BitStream &tgt ) const override;
    void serializefrom( BitStream &src ) override;
    // [[ev_def:field]]
    QString m_error;
    EVENT_IMPL(GameEntryError)
};

// [[ev_def:type]]
class DeleteAcknowledged : public GameLinkEvent
{
public:
    DeleteAcknowledged():GameLinkEvent(GameEventTypes::evDeleteAcknowledged)
    {}
    void serializeto( BitStream &tgt ) const override;
    void serializefrom( BitStream &) override {}
    EVENT_IMPL(DeleteAcknowledged)
};


// [[ev_def:type]]
class ServerReconfigured : public InternalEvent
{
public:
    ServerReconfigured():InternalEvent(GameEventTypes::evServerReconfigured) {}
    EVENT_IMPL(ServerReconfigured)
};
} // end of SEGSEvents namespace
