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

void GameDBSyncService::startup()
{
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

    for(Entity * e : ref_entity_mgr.m_live_entlist)
    {
        // update the player characters in DB every n seconds
        if ((int)tick_timer.sec() % m_update_interval == 0)
        {
            switch(e->m_db_store_flags)
            {
            case (uint32_t)DbStoreFlags::Gui: sendGuiUpdateToHandler(e); break;
            case (uint32_t)DbStoreFlags::Options: sendOptionsUpdateToHandler(e); break;
            case (uint32_t)DbStoreFlags::Keybinds: sendKeybindsUpdateToHandler(e); break;
            case (uint32_t)DbStoreFlags::PlayerData: sendPlayerUpdateToHandler(e); break;
            case (uint32_t)DbStoreFlags::Full: sendCharacterUpdateToHandler(e); break;
            default: break;
            }
        }
    }
}

void GameDBSyncService::sendGuiUpdateToHandler(const Entity* e)
{
    QString cerealizedGuiData;
    serializeToQString(e->m_player->m_gui, cerealizedGuiData);

    GuiUpdateMessage* msg = new GuiUpdateMessage(
                GuiUpdateData({
                                       e->m_char->getAccountId(),
                                       cerealizedGuiData
                                   }), (uint64_t)1);

    m_db_handler->putq(msg);
    delete msg;
}

void GameDBSyncService::sendOptionsUpdateToHandler(const Entity* e)
{
    QString cerealizedOptionsData;
    serializeToQString(e->m_player->m_options, cerealizedOptionsData);

    OptionsUpdateMessage* msg = new OptionsUpdateMessage(
                OptionsUpdateData({
                                       e->m_char->getAccountId(),
                                       cerealizedOptionsData
                                   }), (uint64_t)1);

    m_db_handler->putq(msg);
    delete msg;
}

void GameDBSyncService::sendKeybindsUpdateToHandler(const Entity* e)
{
    QString cerealizedKeybindsData;
    serializeToQString(e->m_player->m_keybinds, cerealizedKeybindsData);

    KeybindsUpdateMessage* msg = new KeybindsUpdateMessage(
                KeybindsUpdateData({
                                       e->m_char->getAccountId(),
                                       cerealizedKeybindsData
                                   }), (uint64_t)1);

    m_db_handler->putq(msg);
    delete msg;
}

void GameDBSyncService::sendPlayerUpdateToHandler(const Entity* e)
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
                                     e->m_char->getAccountId(),
                                     cerealizedPlayerData
                                 }), (uint64_t)1);

    m_db_handler->putq(msg);
    delete msg;
}

void GameDBSyncService::sendCharacterUpdateToHandler(const Entity* e)
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
                                        e->m_char->getAccountId()
        }), (uint64_t)1);

    m_db_handler->putq(msg);
    delete msg;
}
