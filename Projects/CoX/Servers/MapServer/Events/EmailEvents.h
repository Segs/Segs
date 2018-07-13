/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "Servers/InternalEvents.h"
#include "MapServer/MapClientSession.h"

#ifndef EMAILEVENTS_H
#define EMAILEVENTS_H

enum EmailEventTypes : uint32_t
{
    evEmailHeader,
    evEmailRead,
    evEmailSent,
    evEmailDelete
};

#define ONE_WAY_MESSAGE(name)\
struct name ## Message final : public InternalEvent\
{\
    name ## Data m_data;\
    name ## Message(name ## Data &&d) :  InternalEvent(EmailEventTypes::ev ## name),m_data(d) {}\
};

struct EmailHeaderData
{
    MapClientSession* src;
    int id;
    QString sender;
    QString subject;
    int timestamp;
};
ONE_WAY_MESSAGE(EmailHeader)

#undef ONE_WAY_MESSAGE
#endif
