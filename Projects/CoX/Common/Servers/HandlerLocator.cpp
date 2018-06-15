/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup CommonServers Projects/CoX/Common/Servers
 * @{
 */

#include "HandlerLocator.h"

#include "SEGSEvent.h"
#include "EventProcessor.h"

#include <QDebug>
EventProcessor * HandlerLocator::m_db_sync_handler = nullptr;
EventProcessor * HandlerLocator::m_auth_handler = nullptr;
std::deque<EventProcessor *> HandlerLocator::m_game_servers;
std::deque<EventProcessor *> HandlerLocator::m_game_db_servers;
std::deque<EventProcessor *> HandlerLocator::m_map_servers;

MessageBus * HandlerLocator::m_message_bus=nullptr;
void shutDownAllActiveHandlers()
{
    for(EventProcessor *game_db_handler : HandlerLocator::allGameDBHandlers())
    {
        if(!game_db_handler)
            continue;
        auto warn = qWarning();
        warn<<"Shutting down GameDBSync..";
        while(game_db_handler->thr_count())
        {
            game_db_handler->putq(SEGSEvent::s_ev_finish.shallow_copy());
            game_db_handler->wait();
        }
        warn<<"Done";
    }

    EventProcessor *dbsync=HandlerLocator::getAuthDB_Handler();
    if(dbsync)
    {
        auto warn = qWarning(); // no new line will be appended to ouput untill `warn` is alive
        warn<<"Shutting down DBSync..";
        if(dbsync->thr_count()>0)
        {
            dbsync->putq(SEGSEvent::s_ev_finish.shallow_copy());
            dbsync->wait();
        }
        warn<<"Done";
    }

}

//! @}
