/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "GameCommand.h"
#include "MapEventTypes.h"
#include "Common/GameData/Clue.h"
#include "BitStream.h" // Forward declaration from GameCommand.h but editor shows errors.


namespace SEGSEvents
{
// [[ev_def:type]]
class ClueList final : public GameCommandEvent
{
    public:

    // [[ev_def:field]
    vClueList m_clue_list;
    explicit ClueList() : GameCommandEvent(MapEventTypes::evClueList){}
    ClueList(vClueList clue_list) : GameCommandEvent(MapEventTypes::evClueList)
    {
        m_clue_list = clue_list;
    }

    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 70

        bs.StorePackedBits(1, m_clue_list.size());
        int count = 0;
        for (const Clue &clue : m_clue_list)
        {
            bs.StoreString(clue.m_name);
            bs.StoreString(clue.m_display_name);
            bs.StoreString(clue.m_detail_text);
            bs.StoreString(clue.m_icon_file);
            ++count;
        }
    }

    EVENT_IMPL(ClueList)
};

// [[ev_def:type]]
class SouvenirListHeaders final : public GameCommandEvent
{
    public:

    // [[ev_def:field]
    vSouvenirList m_souvenir_list;
    explicit SouvenirListHeaders() : GameCommandEvent(MapEventTypes::evSouvenirListHeaders){}
    SouvenirListHeaders(vSouvenirList souvenir_list) : GameCommandEvent(MapEventTypes::evSouvenirListHeaders)
    {
        m_souvenir_list = souvenir_list;
    }

    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 71

        bs.StorePackedBits(1, m_souvenir_list.size());
        int count = 0;
        for (const Souvenir &souvenir : m_souvenir_list)
        {
            bs.StoreBits(1, count);
            bs.StoreString(souvenir.m_name);
            bs.StoreString(souvenir.m_icon);
            ++count;
        }
    }

    EVENT_IMPL(SouvenirListHeaders)

};

// [[ev_def:type]]
class SouvenirDetail final : public GameCommandEvent
{
    public:

    // [[ev_def:field]
    Souvenir m_souvenir;
    explicit SouvenirDetail() : GameCommandEvent(MapEventTypes::evSouvenirDetail){}
    SouvenirDetail(Souvenir souvenir) : GameCommandEvent(MapEventTypes::evSouvenirDetail)
    {
        m_souvenir = souvenir;
    }

    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 72
        bs.StorePackedBits(1, m_souvenir.m_idx);
        bs.StoreString(m_souvenir.m_description);

    }

    EVENT_IMPL(SouvenirDetail)
};

// [[ev_def:type]]
    class SouvenirDetailRequest final : public MapLinkEvent
    {

    public:
        // [[ev_def:field]]
        int32_t m_souvenir_idx = 0;
        explicit SouvenirDetailRequest() : MapLinkEvent(MapEventTypes::evSouvenirDetailRequest){}

        void serializeto(BitStream &/*bs*/) const override
        {
            assert(!"SouvenirDetailRequest serializeto");
        }
        void    serializefrom(BitStream &bs)
        {
            m_souvenir_idx = bs.GetPackedBits(1);
            qCDebug(logMapEvents) << "SouvenirDetailRequest Event";
        }

        EVENT_IMPL(SouvenirDetailRequest)
    };

}
