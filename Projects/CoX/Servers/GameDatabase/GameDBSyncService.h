#ifndef GAMEDBSYNCSERVICE_H
#define GAMEDBSYNCSERVICE_H

#include "EventProcessor.h"
#include "Common/Servers/HandlerLocator.h"
#include "Servers/MapServer/EntityStorage.h"
#include "Servers/GameDatabase/GameDBSyncHandler.h"



class GameDBSyncService : public EventProcessor
{
private:
    EntityManager& ref_entity_mgr;
    uint8_t m_game_server_id;
    int m_update_interval = 5;
    GameDBSyncHandler* m_db_handler;

    // EventProcessor interface
    bool per_thread_setup() override;
    void dispatch(SEGSEvent *ev) override;

public:
    GameDBSyncService(EntityManager& em) : ref_entity_mgr(em) {}
    void startup();
    void addPlayer(Entity* e);
    void on_update_timer(const ACE_Time_Value &tick_timer);
    void set_db_handler(uint8_t id);
    void sendGuiUpdateToHandler(const Entity* e);
    void sendOptionsUpdateToHandler(const Entity* e);
    void sendKeybindsUpdateToHandler(const Entity* e);
    void sendPlayerUpdateToHandler(const Entity* e);
    void sendCharacterUpdateToHandler(const Entity* e);
};

#endif // GAMEDBSYNCSERVICE_H
