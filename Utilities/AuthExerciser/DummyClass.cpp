/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup AuthExerciser Projects/CoX/Utilities/AuthExerciser
 * @{
 */

#include "DummyClass.h"

#include "Auth/AuthEvents.h"
#include "Common/AuthProtocol/AuthLink.h"

#include <QtCore/QDebug>
#include <cstring>

using namespace SEGSEvents;
DummyClass::DummyClass()
{

}

void DummyClass::dispatch(Event * ev)
{
    switch(ev->type())
    {
        case evConnect:
        {
            onConnect(static_cast<Connect *>(ev));
            break;
        }
        case evAuthProtocolVersion:
        {
            onServerVersion(static_cast<AuthProtocolVersion *>(ev));
            break;
        }
        case evLoginResponse:
        {
            onLoginResponse(static_cast<LoginResponse *>(ev));
            break;
        }
        case evAuthorizationError:
        {
            onAuthorizationError(static_cast<AuthorizationError *>(ev));
            break;
        }
        case evServerListResponse:
        {
            onServerListResponse(static_cast<ServerListResponse *>(ev));
            break;
        }
        case evServerSelectResponse:
        {
            onServerSelectResponse(static_cast<ServerSelectResponse *>(ev));
            break;
        }
        default:
            qWarning() << "Unhandled event type in dispatch:" << ev->type();
    }
}

void DummyClass::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void DummyClass::serialize_to(std::ostream &/*is*/)
{
    assert(false);
}

void DummyClass::onConnect(Connect * ev)
{
    qInfo() << "ConnectEvent received:" << ev->type();
    AuthLink * lnk = static_cast<AuthLink *>(ev->src());
    m_our_link = lnk;
}

void DummyClass::onServerVersion(AuthProtocolVersion *ev)
{
    qInfo() << "AuthorizationProtocolVersion received:" << ev->type();
    AuthLink * lnk = static_cast<AuthLink *>(ev->src());
    lnk->init_crypto(ev->getProtoVers(), ev->getSeed());
    LoginRequest * login_ptr = new LoginRequest();
    const char * my_login = "my_login";
    const char * my_pass = "my_pass";
    strncpy(login_ptr->m_data.login.data(), my_login, 14);
    strncpy(login_ptr->m_data.password.data(), my_pass, 16);
    lnk->putq(login_ptr);
    qInfo() << "LoginRequest sent:" << login_ptr->type();
}

void DummyClass::onLoginResponse(LoginResponse * ev)
{
    qInfo() << "LoginResponse received:" << ev->type();
    AuthLink * lnk = static_cast<AuthLink *>(ev->src());
    ServerListRequest * server_list_ptr = new ServerListRequest();
    lnk->putq(server_list_ptr);
    qInfo() << "ServerListRequest sent:" << server_list_ptr->type();
}

void DummyClass::onAuthorizationError(AuthorizationError * ev)
{
    qFatal("AuthorizationError received: %s", ev->info());
}

void DummyClass::onServerListResponse(ServerListResponse * ev)
{
    qInfo() << "ServerListResponse received:" << ev->type();
    AuthLink * lnk = static_cast<AuthLink *>(ev->src());
    ServerSelectRequest * server_select_ptr = new ServerSelectRequest();
    server_select_ptr->m_server_id = 1;
    lnk->putq(server_select_ptr);
    qInfo() << "ServerSelectRequest sent:" << server_select_ptr->type();
}

void DummyClass::onServerSelectResponse(ServerSelectResponse *ev)
{
    qInfo() << "ServerSelectResponse received:" << ev->type();
}

//! @}
