#ifndef GAMEDBSYNCSERVICE_H
#define GAMEDBSYNCSERVICE_H

#include "EventProcessor.h"
#include "Common/Servers/HandlerLocator.h"
#include "Servers/GameDatabase/GameDBSyncHandler.h"
#include <QVector>

class GameDBSyncService : public EventProcessor
{
private:
    GameDBSyncHandler* m_db_handler;

    // EventProcessor interface
    bool per_thread_setup() override;
    void dispatch(SEGSEvent *ev) override;

    //void sendGuiUpdateToHandler(Entity* e);
    //void sendOptionsUpdateToHandler(Entity* e);
    //void sendKeybindsUpdateToHandler(Entity* e);
    void on_player_update(PlayerUpdateMessage* msg);
    void on_character_update(CharacterUpdateMessage* msg);

public:
    GameDBSyncService() {};
    void set_db_handler(uint8_t id);
};

#endif // GAMEDBSYNCSERVICE_H
