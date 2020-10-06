/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Components/EventProcessor.h"
#include "Common/Servers/HandlerLocator.h"
#include "Servers/GameDatabase/GameDBSyncHandler.h"
#include <QVector>
namespace SEGSEvents {
struct CharacterUpdateMessage;
struct PlayerUpdateMessage;
struct CostumeUpdateMessage;
}
class GameDBSyncService : public EventProcessor
{
private:
    GameDBSyncHandler* m_db_handler;

    void on_character_update(SEGSEvents::CharacterUpdateMessage* msg);
    void on_player_update(SEGSEvents::PlayerUpdateMessage* msg);
    void on_costume_update(SEGSEvents::CostumeUpdateMessage* msg);

public:
    IMPL_ID(GameDBSyncService)
    GameDBSyncService() {}
    void set_db_handler(uint8_t id);
protected:
    // EventProcessor interface
    bool per_thread_startup() override;
    void dispatch(SEGSEvents::Event *ev) override;
    void serialize_from(std::istream &is) override;
    void serialize_to(std::ostream &is) override;

};
