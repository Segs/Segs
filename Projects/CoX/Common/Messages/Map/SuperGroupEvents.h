/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"
#include "Servers/MapServer/DataHelpers.h"
#include "Logging.h"

class QString;

namespace SEGSEvents
{

// [[ev_def:type]]
class SuperGroupOffer final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    uint32_t m_db_id;
    // [[ev_def:field]]
    QString m_name;

    explicit SuperGroupOffer() : GameCommandEvent(MapEventTypes::evSuperGroupOffer) {}
    SuperGroupOffer(uint32_t &db_id, QString &name) : GameCommandEvent(MapEventTypes::evSuperGroupOffer),
        m_db_id(db_id),
        m_name(name)
    {
    }
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 29

        bs.StoreBits(32,m_db_id);   // sg offeree db_id
        bs.StoreString(m_name);     // sg offerer name
        qCDebug(logSuperGroups) << "SuperGroupOffer";
    }

    EVENT_IMPL(SuperGroupOffer)
};

// [[ev_def:type]]
class SuperGroupResponse final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    bool m_success;
    // [[ev_def:field]]
    Costume m_costume;

    explicit SuperGroupResponse() : GameCommandEvent(MapEventTypes::evSuperGroupResponse) {}
    SuperGroupResponse(bool &success, Costume &costume) : GameCommandEvent(MapEventTypes::evSuperGroupResponse),
        m_success(success),
        m_costume(costume)
    {
    }
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 30
        bs.StoreBits(1, m_success);

        qCDebug(logSuperGroups) << "SuperGroupResponse" << m_success;
        // if successful, send SG costume
        if(m_success)
            m_costume.serializeCostume(bs);
    }

    EVENT_IMPL(SuperGroupResponse)
};

// [[ev_def:type]]
class SuperGroupCostume final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    Costume m_costume;

    explicit SuperGroupCostume() : GameCommandEvent(MapEventTypes::evSuperGroupCostume) {}
    SuperGroupCostume(Costume &costume) : GameCommandEvent(MapEventTypes::evSuperGroupCostume),
        m_costume(costume)
    {
    }
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 31
        // send costume
        m_costume.serializeCostume(bs);
        qCDebug(logSuperGroups) << "SuperGroupCostume";
    }

    EVENT_IMPL(SuperGroupCostume)
};

// [[ev_def:type]]
class RegisterSuperGroup final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    QString m_unknown;

    explicit RegisterSuperGroup() : GameCommandEvent(MapEventTypes::evRegisterSuperGroup) {}
    RegisterSuperGroup(QString &unknown) : GameCommandEvent(MapEventTypes::evRegisterSuperGroup),
        m_unknown(unknown)
    {
    }
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 31
        bs.StoreString(m_unknown); // unknown
        qCDebug(logSuperGroups) << "RegisterSuperGroup:" << m_unknown;
    }

    EVENT_IMPL(RegisterSuperGroup)
};

// [[ev_def:type]]
class SuperGroupMode final : public MapLinkEvent
{
public:
    SuperGroupMode() : MapLinkEvent(MapEventTypes::evSuperGroupMode)
    {
    }
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 53
    }
    void    serializefrom(BitStream &bs)
    {
        qCDebug(logSuperGroups) << "Changing SuperGroup Mode";
    }

    EVENT_IMPL(SuperGroupMode)
};

// [[ev_def:type]]
class CreateSuperGroup final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    SuperGroupData data;

    CreateSuperGroup() : MapLinkEvent(MapEventTypes::evCreateSuperGroup)
    {
    }
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 57
    }
    void    serializefrom(BitStream &bs)
    {
        bs.GetString(data.m_sg_name);
        bs.GetString(data.m_sg_titles[0]);
        bs.GetString(data.m_sg_titles[1]);
        bs.GetString(data.m_sg_titles[2]);
        bs.GetString(data.m_sg_emblem);
        data.m_sg_colors[0] = bs.GetPackedBits(32);
        data.m_sg_colors[1] = bs.GetPackedBits(32);
        qCDebug(logSuperGroups) << "CreateSuperGroup";
    }

    EVENT_IMPL(CreateSuperGroup)
};

// [[ev_def:type]]
class ChangeSuperGroupColors final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    uint32_t m_sg_colors[2] = {0};

    ChangeSuperGroupColors() : MapLinkEvent(MapEventTypes::evChangeSuperGroupColors)
    {
    }
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 58
    }
    void    serializefrom(BitStream &bs)
    {
        m_sg_colors[0] = bs.GetPackedBits(32);
        m_sg_colors[1] = bs.GetPackedBits(32);
        qCDebug(logSuperGroups) << "Changing SuperGroup Colors";
    }

    EVENT_IMPL(ChangeSuperGroupColors)
};

// [[ev_def:type]]
class AcceptSuperGroupChanges final : public MapLinkEvent
{
public:
    AcceptSuperGroupChanges() : MapLinkEvent(MapEventTypes::evAcceptSuperGroupChanges)
    {
    }
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,type()-MapEventTypes::evFirstServerToClient); // 59
    }
    void    serializefrom(BitStream &bs)
    {
        qCDebug(logSuperGroups) << "Accept SuperGroup Changes";
    }

    EVENT_IMPL(AcceptSuperGroupChanges)
};

} // end of SEGSEvents namespace
