#pragma once
#include "AuthEvents.h"
#include "AuthOpcodes.h"

class AuthEventFactory
{
public:
	static AuthLinkEvent *EventForType(eAuthPacketType type);
	static void Destroy(AuthLinkEvent *what);
};