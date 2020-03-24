#include "GameDBSyncService.h"
#include "Common/Servers/HandlerLocator.h"
#include "Common/Servers/MessageBus.h"
#include "Common/Servers/InternalEvents.h"
#include "Messages/GameDatabase/GameDBSyncEvents.h"

using namespace SEGSEvents;

bool GameDBSyncService::per_thread_startup()
{
    //GameDbSyncContext &db_ctx(m_db_context.localData());
    //bool result = db_ctx.loadAndConfigure();
    bool result = true;
    if(!result)
        postGlobalEvent(new ServiceStatusMessage({"GameDbSyncService failed to load/configure",-1},0));
    else
        postGlobalEvent(new ServiceStatusMessage({"GameDbSyncService loaded/configured",0},0));
    return result;
}

void GameDBSyncService::dispatch(Event *ev)
{
    // We are servicing a request from message queue, using dispatchSync as a common processing point.
    // nullptr result means that the given message is one-way
    switch (ev->type())
    {
    case evCharacterUpdateMessage:
        on_character_update(static_cast<CharacterUpdateMessage *>(ev)); break;
    case evPlayerUpdateMessage:
        on_player_update(static_cast<PlayerUpdateMessage *>(ev)); break;
    case evCostumeUpdateMessage:
        on_costume_update(static_cast<CostumeUpdateMessage *>(ev)); break;
    default: assert(false); break;
    }
}

void GameDBSyncService::serialize_from(std::istream &/*is*/)
{
    assert(false);
}

void GameDBSyncService::serialize_to(std::ostream &/*is*/)
{
    assert(false);
}
void GameDBSyncService::set_db_handler(const uint8_t id)
{
    m_db_handler = static_cast<GameDBSyncHandler*>(
                HandlerLocator::getGame_DB_Handler(id));
}

// they are quite literally the 'postman' between MapInstance and DbHandler :)
void GameDBSyncService::on_character_update(CharacterUpdateMessage* msg)
{
    CharacterUpdateMessage* newMsg = new CharacterUpdateMessage(
    {
                    msg->m_data.m_char_name,

                    // Cerealized blobs
                    msg->m_data.m_costume_data,
                    msg->m_data.m_char_data,
                    msg->m_data.m_entity_data,
                    msg->m_data.m_player_data,

                    // plain values
                    msg->m_data.m_supergroup_id,
                    msg->m_data.m_id
    }, uint64_t(1));

    m_db_handler->putq(newMsg);
}

void GameDBSyncService::on_player_update(PlayerUpdateMessage * msg)
{
    PlayerUpdateMessage* newMsg = new PlayerUpdateMessage(
    {
                    msg->m_data.m_id,
                    msg->m_data.m_player_data
                }, uint64_t(1));

    m_db_handler->putq(newMsg);
}

void GameDBSyncService::on_costume_update(CostumeUpdateMessage * msg)
{
    CostumeUpdateMessage* newMsg = new CostumeUpdateMessage(
    {
                    msg->m_data.m_id,
                    msg->m_data.m_costume_data
                }, uint64_t(1));

    m_db_handler->putq(newMsg);
}
