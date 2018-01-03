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
    qInfo() << "DummyClass::dispatch called.";
    qDebug() << ev->type();
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
    }
}

void DummyClass::onConnect(ConnectEvent * ev)
{
    AuthLink * lnk = static_cast<AuthLink *>(ev->src());
    m_our_link = lnk;
}

void DummyClass::onServerVersion(AuthorizationProtocolVersion * ev)
{
    assert(m_our_link != nullptr);
    m_our_link->init_crypto(ev->getProtoVers(), ev->getSeed());
    LoginRequest * login_ptr = new LoginRequest();
    const char * my_login = "my_login";
    const char * my_pass = "my_pass";
    strncpy(login_ptr->login, my_login, 14);
    strncpy(login_ptr->password, my_pass, 16);
    m_our_link->putq(login_ptr);
}

SEGSEvent* DummyClass::dispatchSync(SEGSEvent * ev)
{
    qInfo() << "DummyClass:dispatchSync called.";
    return ev;
}
