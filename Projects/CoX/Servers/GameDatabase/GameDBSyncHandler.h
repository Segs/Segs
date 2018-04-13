#pragma once

#include "GameDBSyncContext.h"
#include "EventProcessor.h"

#include <QThreadStorage>

struct CharacterUpdateMessage;
struct CostumeUpdateMessage;
struct GameAccountRequest;

class GameDBSyncHandler final : public EventProcessor
{
    /// Qt requires each db connection to be created and used in same thread
    /// in case there are more `activated` handlers
    QThreadStorage<GameDbSyncContext> m_db_context;
    // EventProcessor interface
    bool per_thread_setup() override;
    void dispatch(SEGSEvent *ev) override;
    void on_character_update(CharacterUpdateMessage *msg);
    void on_costume_update(CostumeUpdateMessage *msg);
    void on_account_request(GameAccountRequest *msg);
public:
    GameDBSyncHandler(uint8_t id);
};
