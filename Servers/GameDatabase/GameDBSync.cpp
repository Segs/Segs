/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameDatabase Projects/CoX/Servers/GameDatabase
 * @{
 */

#include "GameDBSync.h"

#include "GameDBSyncHandler.h"
#include "HandlerLocator.h"

#include <QDebug>
#include <unordered_map>

static std::unordered_map<uint8_t,GameDBSyncHandler *> s_game_db_handlers;

void startGameDBSync(uint8_t for_game_server_id)
{
    if(s_game_db_handlers[for_game_server_id]!=nullptr)
    {
        qCritical() << "Cannot start db sync service, it's already running";
        return;
    }
    auto handler = new GameDBSyncHandler(for_game_server_id);
    s_game_db_handlers[for_game_server_id] = handler;
    qDebug() << "Starting up Game DBSync service for server"<<for_game_server_id;
    handler->activate();
    HandlerLocator::setGame_DB_Handler(for_game_server_id,handler);

}

//! @}
