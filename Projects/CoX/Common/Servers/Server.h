/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include <QString>
// this class represents very high level interface to all servers
class Server
{
public:
virtual         ~Server() = default;
virtual bool    ReadConfig()=0;
virtual bool    Run(void)=0;
virtual bool    ShutDown(const QString &reason="No particular reason")=0;
};
