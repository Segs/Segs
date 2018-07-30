#ifndef GAMEDBSYNCSERVICE_H
#define GAMEDBSYNCSERVICE_H

#include "EventProcessor.h"
#include "Common/Servers/HandlerLocator.h"
#include "Servers/MapServer/EntityStorage.h"
#include "Servers/GameDatabase/GameDBSyncHandler.h"
#include <QVector>

class GameDBSyncService : public EventProcessor
{
private:
    QVector<Entity *> m_entities;
    GameDBSyncHandler* m_db_handler;

    // EventProcessor interface
    bool per_thread_setup() override;
    void dispatch(SEGSEvent *ev) override;

    void sendGuiUpdateToHandler(Entity* e);
    void sendOptionsUpdateToHandler(Entity* e);
    void sendKeybindsUpdateToHandler(Entity* e);
    void sendPlayerUpdateToHandler(Entity* e);
    void sendCharacterUpdateToHandler(Entity* e);

public:
    GameDBSyncService();

    void on_destroy();
    void set_db_handler(uint8_t id);
    void updateEntity(Entity* e);
    void updateEntities();
    void addPlayer(Entity* e);
    void removePlayer(Entity* e);
};

#endif // GAMEDBSYNCSERVICE_H
