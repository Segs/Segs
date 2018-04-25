/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include "AuthEvents.h"
#include "AuthOpcodes.h"

class AuthEventFactory
{
public:
static  AuthLinkEvent * EventForType(eAuthPacketType type);
static  void            Destroy(AuthLinkEvent *what);
};
