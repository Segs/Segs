/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <QtCore/QLoggingCategory>
#include <QElapsedTimer>

// Declare common logging categories
#define SEGS_DECLARE_LOGGING_CATEGORY(name) \
    extern QLoggingCategory &name();

SEGS_DECLARE_LOGGING_CATEGORY(logLogging)
SEGS_DECLARE_LOGGING_CATEGORY(logKeybinds)
SEGS_DECLARE_LOGGING_CATEGORY(logSettings)
SEGS_DECLARE_LOGGING_CATEGORY(logGUI)
SEGS_DECLARE_LOGGING_CATEGORY(logTeams)
SEGS_DECLARE_LOGGING_CATEGORY(logDB)
SEGS_DECLARE_LOGGING_CATEGORY(logInput)
SEGS_DECLARE_LOGGING_CATEGORY(logPosition)
SEGS_DECLARE_LOGGING_CATEGORY(logOrientation)
SEGS_DECLARE_LOGGING_CATEGORY(logMovement)
SEGS_DECLARE_LOGGING_CATEGORY(logChat)
SEGS_DECLARE_LOGGING_CATEGORY(logInfoMsg)
SEGS_DECLARE_LOGGING_CATEGORY(logEmotes)
SEGS_DECLARE_LOGGING_CATEGORY(logTarget)
SEGS_DECLARE_LOGGING_CATEGORY(logCharSel)
SEGS_DECLARE_LOGGING_CATEGORY(logPlayerSpawn)
SEGS_DECLARE_LOGGING_CATEGORY(logNpcSpawn)
SEGS_DECLARE_LOGGING_CATEGORY(logMapEvents)
SEGS_DECLARE_LOGGING_CATEGORY(logMapXfers)
SEGS_DECLARE_LOGGING_CATEGORY(logSlashCommand)
SEGS_DECLARE_LOGGING_CATEGORY(logDescription)
SEGS_DECLARE_LOGGING_CATEGORY(logFriends)
SEGS_DECLARE_LOGGING_CATEGORY(logMiniMap)
SEGS_DECLARE_LOGGING_CATEGORY(logLFG)
SEGS_DECLARE_LOGGING_CATEGORY(logNPCs)
SEGS_DECLARE_LOGGING_CATEGORY(logAnimations)
SEGS_DECLARE_LOGGING_CATEGORY(logPowers)
SEGS_DECLARE_LOGGING_CATEGORY(logTrades)
SEGS_DECLARE_LOGGING_CATEGORY(logTailor)
SEGS_DECLARE_LOGGING_CATEGORY(logScripts)
SEGS_DECLARE_LOGGING_CATEGORY(logSceneGraph)
SEGS_DECLARE_LOGGING_CATEGORY(logStores)
SEGS_DECLARE_LOGGING_CATEGORY(logTasks)
SEGS_DECLARE_LOGGING_CATEGORY(logRPC)
SEGS_DECLARE_LOGGING_CATEGORY(logAFK)
SEGS_DECLARE_LOGGING_CATEGORY(logConnection)
SEGS_DECLARE_LOGGING_CATEGORY(logMigration)

void    setLoggingFilter();
void    toggleLogging(QString &category);
void    dumpLogging();
#define TIMED_LOG(x,msg) {\
    QDebug log(qDebug());\
    log << msg << "..."; \
    QElapsedTimer timer;\
    timer.start();\
    x;\
    log << "done in"<<float(timer.elapsed())/1000.0f<<"s";\
}
