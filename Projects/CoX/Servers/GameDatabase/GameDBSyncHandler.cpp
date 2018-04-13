#include "GameDBSyncHandler.h"

#include "HandlerLocator.h"
#include "GameDBSyncEvents.h"
#include "MessageBus.h"

bool GameDBSyncHandler::per_thread_setup()
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    bool result = db_ctx.loadAndConfigure();
    if(!result)
        postGlobalEvent(new ServiceStatusMessage({"GameDbSync failed to load/configure",-1}));
    else
        postGlobalEvent(new ServiceStatusMessage({"GameDbSync loaded/configured",0}));
    return result;
}

void GameDBSyncHandler::dispatch(SEGSEvent *ev)
{
    // We are servicing a request from message queue, using dispatchSync as a common processing point.
    // nullptr result means that the given message is one-way
    switch (ev->type())
    {

        case GameDBEventTypes::evCharacterUpdate:
        on_character_update(static_cast<CharacterUpdateMessage *>(ev));
        break;
        case GameDBEventTypes::evCostumeUpdate:
        on_costume_update(static_cast<CostumeUpdateMessage *>(ev));
        break;
        case GameDBEventTypes::evGameAccountRequest:
        on_account_request(static_cast<GameAccountRequest *>(ev));
        break;
    }
}

GameDBSyncHandler::GameDBSyncHandler(uint8_t id)
{
    HandlerLocator::setGame_DB_Handler(id,this);
}



void GameDBSyncHandler::on_character_update(CharacterUpdateMessage *msg)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    db_ctx.performUpdate(msg->m_data);

}

void GameDBSyncHandler::on_costume_update(CostumeUpdateMessage *msg)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    db_ctx.performUpdate(msg->m_data);
}
void GameDBSyncHandler::on_account_request(GameAccountRequest *msg)
{
    GameDbSyncContext &db_ctx(m_db_context.localData());
    GameAccountResponseData resp;
    if(db_ctx.getAccount(msg->m_data,resp))
        msg->src()->putq(new GameAccountResponse(std::move(resp),msg->session_token()));
    else
        msg->src()->putq(new GameDbErrorMessage({"Game db error"},msg->session_token()));

}
