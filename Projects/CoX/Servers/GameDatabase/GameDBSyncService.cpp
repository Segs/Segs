#include "GameDBSyncService.h"
#include "Common/Servers/HandlerLocator.h"
#include "Common/Servers/MessageBus.h"
#include "Common/Servers/InternalEvents.h"

bool GameDBSyncService::per_thread_setup()
{
    //GameDbSyncContext &db_ctx(m_db_context.localData());
    //bool result = db_ctx.loadAndConfigure();
    bool result = true;
    if(!result)
        postGlobalEvent(new ServiceStatusMessage({"GameDbSyncService failed to load/configure",-1}));
    else
        postGlobalEvent(new ServiceStatusMessage({"GameDbSyncService loaded/configured",0}));
    return result;
}

void GameDBSyncService::dispatch(SEGSEvent *ev)
{
    // We are servicing a request from message queue, using dispatchSync as a common processing point.
    // nullptr result means that the given message is one-way
    switch (ev->type())
    {
    default: assert(false); break;
    }
}

void GameDBSyncService::startup()
{

}

void GameDBSyncService::set_db_handler(const uint8_t id)
{
    m_db_handler = static_cast<GameDBSyncHandler*>(
                HandlerLocator::getGame_DB_Handler(id));
}

void GameDBSyncService::on_update_timer(const ACE_Time_Value &tick_timer)
{
    ACE_Guard<ACE_Thread_Mutex> guard_buffer(ref_entity_mgr.getEntitiesMutex());

    for(Entity * e : ref_entity_mgr.m_live_entlist)
    {
        // update the player characters in DB every n seconds
        if ((int)tick_timer.sec() % m_update_interval == 0)
        {
            if (e->hasValidDbStoreFlag())
            {

            }
        }
    }
}
