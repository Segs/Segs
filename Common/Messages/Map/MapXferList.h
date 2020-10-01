/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"

#include "glm/vec3.hpp"
#include <QtCore/QString>

namespace SEGSEvents
{

// [[ev_def:type]]
class MapXferList final : public GameCommandEvent
{
public:
        // [[ev_def:field]]
        bool        m_has_location;
        // [[ev_def:field]]
        glm::vec3   m_location;
        // [[ev_def:field]]
        QString     m_name;
        explicit    MapXferList() : GameCommandEvent(evMapXferList) {}
                    MapXferList(bool has_location, glm::vec3 location, QString name) : GameCommandEvent(evMapXferList),
                        m_has_location(has_location),
                        m_location(location),
                        m_name(name)
                    {
                    }

        void        serializeto(BitStream &bs) const override
                    {
                        bs.StorePackedBits(1, type()-evFirstServerToClient); // 58

                        bs.StoreBits(1, m_has_location);
                        if(m_has_location)
                        {
                            bs.StoreFloat(m_location.x);
                            bs.StoreFloat(m_location.y);
                            bs.StoreFloat(m_location.z);
                        }
                        bs.StoreString(m_name);
                    }
        void        serializefrom(BitStream &/*src*/) override
                    {
                        assert(false);
                    }
        EVENT_IMPL(MapXferList)
};

// [[ev_def:type]]
class MapXferListClose final : public GameCommandEvent
{
public:
                MapXferListClose() : GameCommandEvent(evMapXferListClose)
                {
                }

        void    serializeto(BitStream &bs) const override {
                    bs.StorePackedBits(1, type()-evFirstServerToClient); // 59
                }
        void    serializefrom(BitStream &/*src*/) override
                {
                    assert(false);
                }
        EVENT_IMPL(MapXferListClose)
};

} //end of SEGSEvents namespace
