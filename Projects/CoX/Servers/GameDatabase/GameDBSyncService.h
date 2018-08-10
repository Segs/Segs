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
    void dispatch(SEGSEvents::Event *ev) override;

public:
    GameDBSyncService(EntityManager &em) : ref_entity_mgr(em) {};
    //GameDBSyncService(){};

    void on_update_timer(const ACE_Time_Value &tick_timer);
    void on_destroy();
    void set_db_handler(uint8_t id);
    void updateEntity(Entity* e);
    void updateEntities();
    void addPlayer(Entity* e);
    void removePlayer(Entity* e);
    void sendGuiUpdateToHandler(Entity* e);
    void sendOptionsUpdateToHandler(Entity* e);
    void sendKeybindsUpdateToHandler(Entity* e);
    void sendPlayerUpdateToHandler(Entity* e);
    void sendCharacterUpdateToHandler(Entity* e);
};

#endif // GAMEDBSYNCSERVICE_H
