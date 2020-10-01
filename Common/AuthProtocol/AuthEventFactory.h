/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <stdint.h>

enum eAuthPacketType : uint8_t ;
namespace SEGSEvents {
class AuthLinkEvent;
}

class AuthEventFactory
{
public:
using   EventType = SEGSEvents::AuthLinkEvent;
    
static  EventType * EventForType(eAuthPacketType type);
static  void        Destroy(EventType *what);
};
