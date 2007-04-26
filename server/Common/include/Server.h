/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Server.h 301 2006-12-26 15:50:44Z nemerle $
 */

#pragma once
#include <string>
// this class represents very high level interface to all servers
class Server
{
public:
	virtual ~Server(){};
	virtual bool ReadConfig(const std::string &name)=0;
	virtual bool Run(void)=0;
    virtual bool ShutDown(const std::string &reason="No particular reason")=0;
};
