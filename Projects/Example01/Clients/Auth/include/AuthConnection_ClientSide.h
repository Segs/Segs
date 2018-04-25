/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once

#include "AuthConnection.h"

class AuthObserver;
class AuthConnection_ClientSide : public AuthConnection
{
    public:
        AuthConnection_ClientSide(AuthObserver *ob);
        virtual ~AuthConnection_ClientSide(){}
};
