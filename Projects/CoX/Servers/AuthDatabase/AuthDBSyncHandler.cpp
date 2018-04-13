#include "AuthDBSyncHandler.h"

#include "AuthDBSyncEvents.h"
#include "HandlerLocator.h"
#include "MessageBus.h"

#include <QSqlError> // used to format returned errors
#include <cassert>
bool AuthDBSyncHandler::per_thread_setup()
{
    AuthDbSyncContext &db_ctx(m_db_context.localData());
    bool result = db_ctx.loadAndConfigure();
    if(!result)
        postGlobalEvent(new ServiceStatusMessage({"AuthDBSync failed to load/configure",-1}));
    else
        postGlobalEvent(new ServiceStatusMessage({"AuthDBSync loaded/configured",0}));
    return result;
}

void AuthDBSyncHandler::dispatch(SEGSEvent *ev)
{
    // We are servicing a request from message queue, using dispatchSync as a common processing point.
    // nullptr result means that the given message is one-way
    switch (ev->type())
    {

        case AuthDBEventTypes::evCreateAccount:
        on_create_account(static_cast<CreateAccountMessage *>(ev));
        break;
        case AuthDBEventTypes::evRetrieveAccountRequest:
        on_retrieve_account(static_cast<RetrieveAccountRequest *>(ev));
        break;
        case AuthDBEventTypes::evValidatePasswordRequest:
        on_validate_password(static_cast<ValidatePasswordRequest *>(ev));
        break;
    }
}

void AuthDBSyncHandler::on_create_account(CreateAccountMessage *msg)
{
    AuthDbSyncContext &db_ctx(m_db_context.localData());
    if(!db_ctx.addAccount(msg->m_data))
    {
        msg->src()->putq(new AuthDbErrorMessage({db_ctx.getLastError()->text()}));
    }
}

void AuthDBSyncHandler::on_retrieve_account(RetrieveAccountRequest *msg)
{
    AuthDbSyncContext &db_ctx(m_db_context.localData());
    RetrieveAccountResponseData resp;
    if(!db_ctx.retrieveAccount(msg->m_data,resp))
    {
        resp.mark_as_missing();
    }
    if(db_ctx.getLastError())
        msg->src()->putq(new AuthDbErrorMessage({db_ctx.getLastError()->text()}));
    else
        msg->src()->putq(new RetrieveAccountResponse(std::move(resp),msg->session_token()));

}
void AuthDBSyncHandler::on_validate_password(ValidatePasswordRequest *msg)
{
    AuthDbSyncContext &db_ctx(m_db_context.localData());
    ValidatePasswordResponseData res;
    if(db_ctx.getPasswordValidity(msg->m_data,res))
    {
        msg->src()->putq(new ValidatePasswordResponse(std::move(res),msg->session_token()));
    }
    msg->src()->putq(new AuthDbErrorMessage({db_ctx.getLastError()->text()}));
}


AuthDBSyncHandler::AuthDBSyncHandler()
{
    HandlerLocator::setAuthDB_Handler(this);
}
