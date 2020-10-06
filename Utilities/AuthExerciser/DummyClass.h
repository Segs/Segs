/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Components/EventProcessor.h"

class AuthLink;

namespace SEGSEvents
{
class Connect;
class AuthProtocolVersion;
class LoginResponse;
class AuthorizationError;
class ServerListResponse;
class ServerSelectResponse;
}
class DummyClass : public EventProcessor
{
public:
    IMPL_ID(DummyClass)
    DummyClass();

    // EventProcessor interface
    void        dispatch(SEGSEvents::Event *ev) override;
protected:
    void        serialize_from(std::istream &is) override;
    void        serialize_to(std::ostream &is) override;

protected:
    void        onConnect(SEGSEvents::Connect * ev);
    void        onServerVersion(SEGSEvents::AuthProtocolVersion * ev);
    void        onLoginResponse(SEGSEvents::LoginResponse *ev);
    void        onAuthorizationError(SEGSEvents::AuthorizationError * ev);
    void        onServerListResponse(SEGSEvents::ServerListResponse * ev);
    void        onServerSelectResponse(SEGSEvents::ServerSelectResponse * ev);

    AuthLink *  m_our_link = nullptr;
};
