#include "InterfaceManager.h"

InterfaceManagerC::InterfaceManagerC(void)
{
}

InterfaceManagerC::~InterfaceManagerC(void)
{
}

AdminServerInterface *InterfaceManagerC::get(const ServerHandle<IAdminServer> &) const
{
    return 0;
}
AuthServerInterface *InterfaceManagerC::get(const ServerHandle<IAuthServer> &) const
{
    return 0;
}
