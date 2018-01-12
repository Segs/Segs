#pragma once

#include <EventProcessor.h>

class AuthLink;

class DummyClass : public EventProcessor
{
public:
    DummyClass();

    // EventProcessor interface
    void        dispatch(SEGSEvent *ev) override;
    SEGSEvent * dispatchSync(SEGSEvent *ev) override;

protected:
    void        onConnect(class ConnectEvent * ev);
    void        onServerVersion(class AuthorizationProtocolVersion * ev);
    void        onLoginResponse(class LoginResponse *ev);
    void        onAuthorizationError(class AuthorizationError * ev);
    void        onServerListResponse(class ServerListResponse * ev);
    void        onServerSelectResponse(class ServerSelectResponse * ev);

    AuthLink *  m_our_link = nullptr;
};
