/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "AuthDBSyncContext.h"
#include "Components/EventProcessor.h"

#include <QThreadStorage>

namespace SEGSEvents
{
struct CreateAccountMessage;
struct RetrieveAccountRequest;
struct ValidatePasswordRequest;
}

class AuthDBSyncHandler final : public EventProcessor
{
    /// Qt requires each db connection to be created and used in same thread
    /// in case there are more `activated` handlers
    QThreadStorage<AuthDbSyncContext> m_db_context;
    // EventProcessor interface
    bool per_thread_startup() override;
    void dispatch(SEGSEvents::Event *ev) override;
    void serialize_from(std::istream &is) override;
    void serialize_to(std::ostream &is) override;

    // Event handlers
    void on_create_account(SEGSEvents::CreateAccountMessage *msg);
    void on_retrieve_account(SEGSEvents::RetrieveAccountRequest *msg);
    void on_validate_password(SEGSEvents::ValidatePasswordRequest *msg);
public:
    IMPL_ID(AuthDBSyncHandler)
    AuthDBSyncHandler();
};
