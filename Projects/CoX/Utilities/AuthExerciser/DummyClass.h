/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <EventProcessor.h>

class AuthLink;

namespace SEGSEvents
{
class ConnectEvent;
class AuthProtocolVersion;
class LoginResponse;
class AuthorizationError;
class ServerListResponse;
class ServerSelectResponse;
}
class DummyClass : public EventProcessor
{
public:
    DummyClass();

    // EventProcessor interface
    void        dispatch(SEGSEvents::Event *ev) override;

protected:
    void        onConnect(SEGSEvents::ConnectEvent * ev);
    void        onServerVersion(SEGSEvents::AuthProtocolVersion * ev);
    void        onLoginResponse(SEGSEvents::LoginResponse *ev);
    void        onAuthorizationError(SEGSEvents::AuthorizationError * ev);
    void        onServerListResponse(SEGSEvents::ServerListResponse * ev);
    void        onServerSelectResponse(SEGSEvents::ServerSelectResponse * ev);

    AuthLink *  m_our_link = nullptr;
};
