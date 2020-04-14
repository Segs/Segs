/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "GameCommand.h"
#include "MapEventTypes.h"
#include "Common/GameData/Contact.h"

namespace SEGSEvents
{
    // [[ev_def:type]]
    class ContactStatusList final : public GameCommandEvent
    {

    public:
        // [[ev_def:field]
        vContactList m_contact_list;
        explicit ContactStatusList() : GameCommandEvent(MapEventTypes::evContactStatusList){}
        ContactStatusList(vContactList contact_list) : GameCommandEvent(MapEventTypes::evContactStatusList)
        {
            m_contact_list = contact_list;
        }

        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 43
            bs.StorePackedBits(1, m_contact_list.size() - 1); // svr_contacts_total_also_maybe
            bs.StorePackedBits(1, m_contact_list.size()); // svr_contacts_total

            int count = 0;
            for(const Contact &contact : m_contact_list)
            {
                bs.StorePackedBits(1, count);
                bs.StoreString(contact.m_name);
                bs.StoreString(contact.m_location_description);
                bs.StorePackedBits(12, contact.m_npc_id);
                bs.StorePackedBits(1, contact.m_contact_idx); // m_contact_idx count
                bs.StorePackedBits(1, contact.m_task_index);
                bs.StoreBits(1, contact.m_notify_player);
                bs.StoreBits(1, contact.m_can_use_cell);
                bs.StorePackedBits(1, contact.m_current_standing);
                bs.StorePackedBits(1, contact.m_friend_threshold);
                bs.StorePackedBits(1, contact.m_confidant_threshold);
                bs.StorePackedBits(1, contact.m_complete_threshold);
                bs.StoreBits(1, contact.m_has_location);

                if(contact.m_has_location)
                {
                    bs.StoreFloat(contact.m_location.location.x);
                    bs.StoreFloat(contact.m_location.location.y);
                    bs.StoreFloat(contact.m_location.location.z);
                    bs.StoreString(contact.m_location.m_location_map_name);
                    bs.StoreString(contact.m_location.m_location_name);
                }

                 qCDebug(logMapEvents) << "ContactStatusList Event serializeTo. currentStanding: " << contact.m_current_standing << " contactIdx: " <<contact.m_contact_idx;
                ++count;
            }
        }

        EVENT_IMPL(ContactStatusList)
    };

     // [[ev_def:type]]
    class ContactSelect final : public GameCommandEvent
    {
    public:
        // [[ev_def:field]]
        Contact selected_contact;
        explicit ContactSelect() : GameCommandEvent(MapEventTypes::evContactSelect){}
        ContactSelect(Contact contact) : GameCommandEvent(MapEventTypes::evContactSelect)
        {
            selected_contact = contact;
        }

        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 44?
            bs.StorePackedBits(1, selected_contact.m_contact_idx);

        }

        EVENT_IMPL(ContactSelect)
    };

// [[ev_def:type]]
    class ReceiveContactStatus final : public MapLinkEvent
    {

    public:
        // [[ev_def:field]]
        int32_t m_srv_idx = 0;
        explicit ReceiveContactStatus() : MapLinkEvent(MapEventTypes::evReceiveContactStatus){}

        void serializeto(BitStream &/*bs*/) const override
        {
            assert(!"ReceiveContactStatus serializeto");
        }
        void serializefrom(BitStream &bs) override
        {
            m_srv_idx = bs.GetPackedBits(1);
            qCDebug(logMapEvents) << "ReceiveContactStatus Event";
        }

        EVENT_IMPL(ReceiveContactStatus)

    };
}
