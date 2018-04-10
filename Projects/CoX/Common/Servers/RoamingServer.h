/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include <ace/INET_Addr.h>
#include <QString>

#include "Server.h"
/**
  * class RoamingServer
  * An abstract class that knows how to connect to World cluster (AuthServer+AdminServer+GameServers)
  * Later on we might want to create other server types ( AiServer,PhysicsServer maybe ? )
  */
class AuthServerInterface;
class RoamingServer : public Server
{
public:
virtual         ~RoamingServer() = default;

virtual bool    ReadConfig()=0;
virtual bool    Run(void) = 0;
virtual bool    ShutDown(const QString &reason)=0;
virtual bool    Online()=0;

private:
        ACE_INET_Addr           m_authaddr;
        QString                 m_passw;
};
