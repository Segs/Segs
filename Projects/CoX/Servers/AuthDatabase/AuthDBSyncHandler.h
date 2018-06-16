/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "AuthDBSyncContext.h"
#include "EventProcessor.h"

#include <QThreadStorage>

struct CreateAccountMessage;
struct RetrieveAccountRequest;
struct ValidatePasswordRequest;

class AuthDBSyncHandler final : public EventProcessor
{
    /// Qt requires each db connection to be created and used in same thread
    /// in case there are more `activated` handlers
    QThreadStorage<AuthDbSyncContext> m_db_context;
    // EventProcessor interface
    bool per_thread_setup() override;
    void dispatch(SEGSEvent *ev) override;

    // Event handlers
    void on_create_account(CreateAccountMessage *msg);
    void on_retrieve_account(RetrieveAccountRequest *msg);
    void on_validate_password(ValidatePasswordRequest *msg);
public:
    AuthDBSyncHandler();
};
