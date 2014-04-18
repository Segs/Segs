/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include <string>
#include <ace/INET_Addr.h>

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
virtual         ~RoamingServer(){}

virtual bool    ReadConfig(const std::string &configpath)=0;
virtual bool    Run(void) = 0;
virtual bool    ShutDown(const std::string &reason)=0;
virtual bool    Online()=0;

protected:
        AuthServerInterface *   getAuthServer();
private:
        ACE_INET_Addr           m_authaddr;
        std::string             m_passw;
};
