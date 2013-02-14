/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once

#include "GameProtocolHandler.h"

class ServerCommandHandler : public LinkCommandHandler
{
public:
	bool ReceiveControlPacket(const ControlPacket *pak);
	//! this is abstract, because each specific server can use different Client class
	virtual void			setClient(IClient *cl) = 0; //! used to link a handler with underlying client object
};
