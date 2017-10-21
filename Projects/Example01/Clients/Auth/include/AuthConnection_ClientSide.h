#pragma once

#include "AuthConnection.h"
class AuthObserver;
class AuthConnection_ClientSide : public AuthConnection
{
public:
    AuthConnection_ClientSide(AuthObserver *ob);
    virtual ~AuthConnection_ClientSide(){}
};
