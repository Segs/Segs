#include "GameDBSyncService.h"
#include "Common/Servers/HandlerLocator.h"
#include "Common/Servers/MessageBus.h"
#include "Common/Servers/InternalEvents.h"
#include "GameData/serialization_common.h"
#include "GameDBSyncEvents.h"
#include "Character.h"
#include "GameData/chardata_serializers.h"
#include "GameData/entitydata_serializers.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/playerdata_serializers.h"
#include "GameData/gui_serializers.h"
#include "GameData/keybind_serializers.h"
#include "GameData/clientoptions_serializers.h"

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

void GameDBSyncService::set_db_handler(const uint8_t id)
{
    m_db_handler = static_cast<GameDBSyncHandler*>(
                HandlerLocator::getGame_DB_Handler(id));
}

void GameDBSyncService::addPlayer(Entity* e)
{
    ref_entity_mgr.InsertPlayer(e);
}

void GameDBSyncService::on_update_timer(const ACE_Time_Value &tick_timer)
{
    ACE_Guard<ACE_Thread_Mutex> guard_buffer(ref_entity_mgr.getEntitiesMutex());

    // update the player characters in DB every n seconds
    if ((int)tick_timer.sec() % m_update_interval == 0)
        updateEntities();
}

void GameDBSyncService::on_destroy()
{
    updateEntities();
}

void GameDBSyncService::updateEntities()
{
    for(Entity * e : ref_entity_mgr.m_live_entlist)
    {
        // These are ordered based on how much data is sent depending on those flags
        if (e->m_db_store_flags & uint32_t(DbStoreFlags::Full))
        {
            sendCharacterUpdateToHandler(e);
            continue;
        }

        // Full character update and PlayerData update encompass Gui, Options and Keybinds already
        // And so, we should just not check for the three below anymore if this is true
        if (e->m_db_store_flags & uint32_t(DbStoreFlags::PlayerData))
        {
            sendPlayerUpdateToHandler(e);
            continue;
        }
    }
}

void GameDBSyncService::sendPlayerUpdateToHandler(Entity* e)
{
    QString cerealizedPlayerData;

    PlayerData playerData = PlayerData({
                e->m_player->m_gui,
                e->m_player->m_keybinds,
                e->m_player->m_options
                });

    serializeToQString(playerData, cerealizedPlayerData);

    PlayerUpdateMessage* msg = new PlayerUpdateMessage(
                PlayerUpdateData({
                                     e->m_char->m_db_id,
                                     cerealizedPlayerData
                                 }), (uint64_t)1);

    m_db_handler->putq(msg);
    unmarkEntityForDbStore(e, DbStoreFlags::PlayerData);
}

void GameDBSyncService::sendCharacterUpdateToHandler(Entity* e)
{
    QString cerealizedCharData, cerealizedEntityData, cerealizedPlayerData;

    PlayerData playerData = PlayerData({
                e->m_player->m_gui,
                e->m_player->m_keybinds,
                e->m_player->m_options
                });

    serializeToQString(e->m_char->m_char_data, cerealizedCharData);
    serializeToQString(e->m_entity_data, cerealizedEntityData);
    serializeToQString(playerData, cerealizedPlayerData);

    CharacterUpdateMessage* msg = new CharacterUpdateMessage(
                CharacterUpdateData({
                                        e->m_char->getName(),

                                        // cerealized blobs
                                        cerealizedCharData,
                                        cerealizedEntityData,
                                        cerealizedPlayerData,

                                        // plain values
                                        e->m_char->getCurrentCostume()->m_body_type,
                                        e->m_char->getCurrentCostume()->m_height,
                                        e->m_char->getCurrentCostume()->m_physique,
                                        (uint32_t)e->m_supergroup.m_SG_id,
                                        e->m_char->m_db_id
        }), (uint64_t)1);

    m_db_handler->putq(msg);
    unmarkEntityForDbStore(e, DbStoreFlags::Full);
}
