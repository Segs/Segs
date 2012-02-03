/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

// Inclusion guards
#pragma once
#ifndef MAPSERVERINTERFACE_H
#define MAPSERVERINTERFACE_H

#include <ace/INET_Addr.h>
#include <string>
#include "Base.h"
#include "RoamingServer.h"
class MapServer;
class EventProcessor;
// this is a proxy for calling MapServer services
class IMapServer : public RoamingServer
{
public:

virtual const ACE_INET_Addr &   getAddress()=0;
virtual EventProcessor *        event_target()=0;
};
class MapServerInterface : public Server
{
public:
                                MapServerInterface(IMapServer *mi) : m_instance(mi){}
                                ~MapServerInterface(void){}

	bool                    ReadConfig(const std::string &name); // later name will be used to read GameServer specific configuration
	bool                    Run(void);
	bool                    ShutDown(const std::string &reason);
	bool                    isLocal(){return true;} // this method returns true if this interface is a local ( same process )
	bool                    Online();
	EventProcessor *        event_target();
        const ACE_INET_Addr &   getAddress();

protected:
        IMapServer *            m_instance;
};

#endif // MAPSERVERINTERFACE_H
