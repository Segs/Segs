#include "InterfaceManager.h"

InterfaceManagerC::InterfaceManagerC(void)
{
}

InterfaceManagerC::~InterfaceManagerC(void)
{
}

AdminServerInterface *InterfaceManagerC::get(const ServerHandle<AdminServer> &h_serv) const
{
	return 0;
}
AuthServerInterface *InterfaceManagerC::get(const ServerHandle<AuthServer> &h_serv) const
{
	return 0;
}
