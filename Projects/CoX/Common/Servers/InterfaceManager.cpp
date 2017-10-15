#include "InterfaceManager.h"

AdminServerInterface *InterfaceManagerC::get(const ServerHandle<IAdminServer> &) const
{
    return nullptr;
}
AuthServerInterface *InterfaceManagerC::get(const ServerHandle<IAuthServer> &) const
{
    return nullptr;
}
