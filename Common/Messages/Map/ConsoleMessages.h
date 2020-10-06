/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "GameCommand.h"
#include "MapEventTypes.h"
#include "Components/BitStream.h" // Forward declaration from GameCommand.h but editor shows errors.


namespace SEGSEvents
{
// [[ev_def:type]]
class ConsoleOutput final : public GameCommandEvent
{
    public:

    // [[ev_def:field]
    QString m_msg;
    explicit ConsoleOutput() : GameCommandEvent(MapEventTypes::evConsoleOutput){}
    ConsoleOutput(QString msg) : GameCommandEvent(MapEventTypes::evConsoleOutput)
    {
        m_msg = msg;
    }

    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 17
        bs.StoreString(m_msg);
    }

    EVENT_IMPL(ConsoleOutput)
};

// [[ev_def:type]]
class ConsolePrint final : public GameCommandEvent
{
    public:

    // [[ev_def:field]
    QString m_msg;
    explicit ConsolePrint() : GameCommandEvent(MapEventTypes::evConsolePrint){}
    ConsolePrint(QString msg) : GameCommandEvent(MapEventTypes::evConsolePrint)
    {
        m_msg = msg;
    }

    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 19
        bs.StoreString(m_msg);
    }

    EVENT_IMPL(ConsolePrint)

};
}
