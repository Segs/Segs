/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup AuthDatabase Projects/CoX/Servers/AuthDatabase
 * @{
 */

#include "AuthDBSyncHandler.h"

#include "Messages/AuthDatabase/AuthDBSyncEvents.h"
#include "HandlerLocator.h"
#include "MessageBus.h"

#include <QSqlError> // used to format returned errors
#include <cassert>

using namespace SEGSEvents;

bool AuthDBSyncHandler::per_thread_startup()
{
    AuthDbSyncContext &db_ctx(m_db_context.localData());
    bool result = db_ctx.loadAndConfigure();
    if(!result)
        postGlobalEvent(new ServiceStatusMessage({"AuthDBSync failed to load/configure",-1},0));
    else
        postGlobalEvent(new ServiceStatusMessage({"AuthDBSync loaded/configured",0},0));
    return result;
}

void AuthDBSyncHandler::dispatch(Event *ev)
{
    // We are servicing a request from message queue, using dispatchSync as a common processing point.
    // nullptr result means that the given message is one-way
    switch (ev->type())
    {

        case AuthDBEventTypes::evCreateAccountMessage:
            on_create_account(static_cast<CreateAccountMessage *>(ev));
            break;
        case AuthDBEventTypes::evRetrieveAccountRequest:
            on_retrieve_account(static_cast<RetrieveAccountRequest *>(ev));
            break;
        case AuthDBEventTypes::evValidatePasswordRequest:
            on_validate_password(static_cast<ValidatePasswordRequest *>(ev));
            break;
        default:
            assert(false);
            break;
    }
}

void AuthDBSyncHandler::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void AuthDBSyncHandler::serialize_to(std::ostream &/*os*/)
{
    assert(false);
}

void AuthDBSyncHandler::on_create_account(CreateAccountMessage *msg)
{
    AuthDbSyncContext &db_ctx(m_db_context.localData());
    if(!db_ctx.addAccount(msg->m_data))
    {
        if(msg->src() != nullptr)
            msg->src()->putq(new AuthDbStatusMessage({db_ctx.getLastError()->text()},msg->session_token()));
    }
    else
    {
        if(msg->src() != nullptr)
            msg->src()->putq(new AuthDbStatusMessage({"OK"},msg->session_token()));
    }
}

void AuthDBSyncHandler::on_retrieve_account(RetrieveAccountRequest *msg)
{
    AuthDbSyncContext &db_ctx(m_db_context.localData());
    RetrieveAccountResponseData resp;

    if(!db_ctx.retrieveAccountAndCheckPassword(msg->m_data, resp))
    {
        resp.mark_as_missing();
    }

    if(db_ctx.getLastError())
        msg->src()->putq(new AuthDbStatusMessage({db_ctx.getLastError()->text()},msg->session_token()));
    else
        msg->src()->putq(new RetrieveAccountResponse(std::move(resp), msg->session_token()));
}

void AuthDBSyncHandler::on_validate_password(ValidatePasswordRequest *msg)
{
    AuthDbSyncContext &db_ctx(m_db_context.localData());
    ValidatePasswordResponseData res;
    if(db_ctx.getPasswordValidity(msg->m_data,res))
    {
        msg->src()->putq(new ValidatePasswordResponse(std::move(res),msg->session_token()));
    }
    msg->src()->putq(new AuthDbStatusMessage({db_ctx.getLastError()->text()},msg->session_token()));
}

AuthDBSyncHandler::AuthDBSyncHandler()
{
    HandlerLocator::setAuthDB_Handler(this);
}

//! @}
