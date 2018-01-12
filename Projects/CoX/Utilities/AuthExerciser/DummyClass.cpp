#include "DummyClass.h"

#include "AuthProtocol/AuthEvents.h"
#include "AuthProtocol/AuthLink.h"

#include <QtCore/QDebug>
#include <cstring>

DummyClass::DummyClass()
{

}

void DummyClass::dispatch(SEGSEvent * ev)
{
    switch(ev->type())
    {
        case SEGS_EventTypes::evConnect:
        {
            onConnect(static_cast<ConnectEvent *>(ev));
            break;
        }
        case evAuthProtocolVersion:
        {
            onServerVersion(static_cast<AuthorizationProtocolVersion *>(ev));
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

void DummyClass::onConnect(ConnectEvent * ev)
{
    qInfo() << "ConnectEvent received:" << ev->type();
    AuthLink * lnk = static_cast<AuthLink *>(ev->src());
    m_our_link = lnk;
}

void DummyClass::onServerVersion(AuthorizationProtocolVersion * ev)
{
    qInfo() << "AuthorizationProtocolVersion received:" << ev->type();
    AuthLink * lnk = static_cast<AuthLink *>(ev->src());
    lnk->init_crypto(ev->getProtoVers(), ev->getSeed());
    LoginRequest * login_ptr = new LoginRequest();
    const char * my_login = "my_login";
    const char * my_pass = "my_pass";
    strncpy(login_ptr->login, my_login, 14);
    strncpy(login_ptr->password, my_pass, 16);
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
    qFatal("AuthorizationError received: " + ev->type());
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

SEGSEvent* DummyClass::dispatchSync(SEGSEvent * ev)
{
    qInfo() << "DummyClass:dispatchSync called.";
    return ev;
}
