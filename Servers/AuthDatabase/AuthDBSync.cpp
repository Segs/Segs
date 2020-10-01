/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup AuthDatabase Projects/CoX/Servers/AuthDatabase
 * @{
 */

#include "AuthDBSync.h"

#include "AuthDBSyncHandler.h"

#include <QDebug>
static AuthDBSyncHandler *s_db_sync_handler=nullptr;
void startAuthDBSync()
{
    if(s_db_sync_handler!=nullptr)
    {
        qCritical() << "Cannot start db sync service, it's already running";
        return;
    }
    s_db_sync_handler = new AuthDBSyncHandler;
    s_db_sync_handler->activate();
}

//! @}
