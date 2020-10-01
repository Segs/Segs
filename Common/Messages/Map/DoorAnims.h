/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEvents.h"

namespace SEGSEvents
{

    // [[ev_def:type]]
    class DoorAnimStart : public GameCommandEvent
    {
    public:
        // [[ev_def:field]]
        glm::vec3 m_entry_pos;
        // [[ev_def:field]]
        glm::vec3 m_target_pos;
        // [[ev_def:field]]
        bool m_has_anims;
        // [[ev_def:field]]
        QString m_seq_state;

        explicit DoorAnimStart() : GameCommandEvent(evDoorAnimStart) {}
        DoorAnimStart(glm::vec3 entry, glm::vec3 target, bool has_anims, QString seq_state) : GameCommandEvent(evDoorAnimStart),
            m_entry_pos(entry),
            m_target_pos(target),
            m_has_anims(has_anims),
            m_seq_state(seq_state)
        {
        }

        // SerializableEvent interface
        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1,type()-evFirstServerToClient); // pkt 52
            bs.StoreFloat(m_entry_pos.x);
            bs.StoreFloat(m_entry_pos.y);
            bs.StoreFloat(m_entry_pos.z);
            bs.StoreFloat(m_target_pos.x);
            bs.StoreFloat(m_target_pos.y);
            bs.StoreFloat(m_target_pos.z);

            bs.StoreBits(1, m_has_anims);
            if(m_has_anims)
                bs.StoreString(m_seq_state);
        }
        EVENT_IMPL(DoorAnimStart)
    };

    // [[ev_def:type]]
    class DoorAnimExit : public GameCommandEvent
    {
    public:
        // [[ev_def:field]]
        bool m_force_move;

        explicit DoorAnimExit() : GameCommandEvent(evDoorAnimExit) {}
        DoorAnimExit(bool force_move) : GameCommandEvent(evDoorAnimExit),
            m_force_move(force_move)
        {
        }

        // SerializableEvent interface
        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1,type()-evFirstServerToClient); // pkt 53
            bs.StoreBits(1, m_force_move);
        }
        EVENT_IMPL(DoorAnimExit)
    };

    // [[ev_def:type]]
    class EnterDoor final : public MapLinkEvent
    {
    public:
        // [[ev_def:field]]
        bool no_location;
        // [[ev_def:field]]
        glm::vec3 location;
        // [[ev_def:field]]
        QString name;

        EnterDoor():MapLinkEvent(MapEventTypes::evEnterDoor)
        {}
        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1,9);
        }
        void serializefrom(BitStream &bs) override
        {
            no_location = bs.GetBits(1);
            if(!no_location)
            {
                location.x = bs.GetFloat();
                location.y = bs.GetFloat();
                location.z = bs.GetFloat();
            }
            bs.GetString(name);
        }
        EVENT_IMPL(EnterDoor)
    };

    // [[ev_def:type]]
    class HasEnteredDoor final : public MapLinkEvent
    {
    public:
        HasEnteredDoor():MapLinkEvent(MapEventTypes::evHasEnteredDoor)
        {}
        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1,13);
        }
        void serializefrom(BitStream &/*bs*/) override
        {
            // nothing received from client
        }
        EVENT_IMPL(HasEnteredDoor)
    };

} // end of namespace SEGSEvents
