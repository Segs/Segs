/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 
 */
#include <ace/INET_Addr.h>
#include "EventProcessor.h"
#include "GameEvents.h"
#include "crud_link.inl"
template
class CRUDLink<GameEventFactory>;