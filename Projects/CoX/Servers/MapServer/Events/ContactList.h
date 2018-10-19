/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "GameCommand.h"
#include "MapEventTypes.h"
#include "Contact.h"

namespace SEGSEvents
{
    // [[ev_def:type]]
    class ContactStatusList : public GameCommandEvent
    {

    public:
        // [[ev_def:field]
        std::vector<Contact> m_contact_list;
        explicit ContactStatusList() : GameCommandEvent(MapEventTypes::evContactStatusList){}
        ContactStatusList(Contact contact) : GameCommandEvent(MapEventTypes::evContactStatusList)
        {
            m_contact_list.push_back(contact);
        }

        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1, type()-evFirstServerToClient); // 43
            bs.StorePackedBits(1, m_contact_list.size()); // svr_contacts_total_also_maybe
            bs.StorePackedBits(1, m_contact_list.size()); // svr_contacts_total

            int count = 0;
            for(const Contact &contact : m_contact_list)
            {
                bs.StorePackedBits(1, count);
                bs.StoreString(contact.m_name);
                bs.StoreString(contact.m_location_description);
                bs.StorePackedBits(12, contact.m_npc_id);
                bs.StorePackedBits(1, contact.m_handle);
                bs.StorePackedBits(1, contact.m_task_index);
                bs.StoreBits(1, contact.m_notify_player);
                bs.StoreBits(1, contact.m_can_use_cell);
                bs.StorePackedBits(1, contact.m_current_standing);
                bs.StorePackedBits(1, contact.m_friend_threshold);
                bs.StorePackedBits(1, contact.m_confidant_threshold);
                bs.StorePackedBits(1, contact.m_complete_threshold);
                bs.StoreBits(1, contact.m_has_location);

                bs.StoreFloat(contact.m_location.location.x);
                bs.StoreFloat(contact.m_location.location.y);
                bs.StoreFloat(contact.m_location.location.z);
                bs.StoreString(contact.m_location_map_name);
                bs.StoreString(contact.m_location_name);
                ++count;
            }
            qCDebug(logMapEvents) << "ContactStatusList Event serializeto";
        }

        void serializefrom(BitStream &bs)
        {
            qCDebug(logMapEvents) << "ContactStatusList Event serializefrom";
        }

        EVENT_IMPL(ContactStatusList)
    };

     // [[ev_def:type]]
    class ContactSelect : public MapLinkEvent
    {
    public:
        explicit ContactSelect() : MapLinkEvent(MapEventTypes::evContactSelect){}

        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1, type()-evFirstServerToClient); // 44?

        }
        void    serializefrom(BitStream &bs)
        {
            qCDebug(logMapEvents) << "ContactSelect Event";
        }

        EVENT_IMPL(ContactSelect)
    };

// [[ev_def:type]]
    class ReceiveContactStatus : public MapLinkEvent
    {

    public:
        // [[ev_def:field]]
        int32_t m_srv_idx = 0;
        explicit ReceiveContactStatus() : MapLinkEvent(MapEventTypes::evReceiveContactStatus){}

        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1, type()-evFirstServerToClient); // 26
            bs.StorePackedBits(12, m_srv_idx);
        }
        void    serializefrom(BitStream &bs)
        {
            m_srv_idx = bs.GetPackedBits(12);
            qCDebug(logMapEvents) << "ReceiveContactStatus Event";
        }

        EVENT_IMPL(ReceiveContactStatus)

    };
}
