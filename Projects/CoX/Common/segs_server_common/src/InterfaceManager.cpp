#include "InterfaceManager.h"

InterfaceManagerC::InterfaceManagerC(void)
{
}

InterfaceManagerC::~InterfaceManagerC(void)
{
}

AdminServerInterface *InterfaceManagerC::get(const ServerHandle<IAdminServer> &h_serv) const
{
	return 0;
}
AuthServerInterface *InterfaceManagerC::get(const ServerHandle<IAuthServer> &h_serv) const
{
	return 0;
}
