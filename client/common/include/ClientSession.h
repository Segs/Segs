#pragma once
// This file is basically a placeholder, until we have better design for client side session management
class IClientSession
{
public:
	virtual void connect()=0;
	virtual void disconnect()=0;
};