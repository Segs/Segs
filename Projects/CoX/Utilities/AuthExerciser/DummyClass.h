/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <EventProcessor.h>

class AuthLink;

class DummyClass : public EventProcessor
{
public:
    DummyClass();

    // EventProcessor interface
    void        dispatch(SEGSEvent *ev) override;

protected:
    void        onConnect(class ConnectEvent * ev);
    void        onServerVersion(class AuthorizationProtocolVersion * ev);
    void        onLoginResponse(class LoginResponse *ev);
    void        onAuthorizationError(class AuthorizationError * ev);
    void        onServerListResponse(class ServerListResponse * ev);
    void        onServerSelectResponse(class ServerSelectResponse * ev);

    AuthLink *  m_our_link = nullptr;
};
