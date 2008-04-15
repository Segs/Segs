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
#ifndef SERVERHANDLE_H
#define SERVERHANDLE_H

#include <string>
#include <ace/INET_Addr.h>
/**
  * class ServerHandle
  * This class main use is construction of *ServerInterface's.
  * And passing server references across network.
  */

template<class SERVER_CLASS>
class ServerHandle
{
public:

    ServerHandle(SERVER_CLASS *impl );
	ServerHandle(const ACE_INET_Addr &addr,int id);
	virtual ~ServerHandle(){};
private:
    ACE_INET_Addr m_address; //! Address of remote host.
    int m_id; //! Remote host uses this to identify it's servers.
    SERVER_CLASS * m_server; //! Set only if the handle is for same process server (i.e. thread)

	void setAddress( ACE_INET_Addr new_var );
    ACE_INET_Addr getAddress ( );
    void setId ( int new_var );
    int getId ( );
    void setServer ( SERVER_CLASS * new_var );
    SERVER_CLASS * getServer ( );
};

#endif // SERVERHANDLE_H
