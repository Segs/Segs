/*
 * Super Entity Game Server
 * http://github.com/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include <QtCore/QLoggingCategory>

// Declare common logging categories
#define SEGS_DECLARE_LOGGING_CATEGORY(name) \
    extern QLoggingCategory &name();

SEGS_DECLARE_LOGGING_CATEGORY(logKeybinds)
SEGS_DECLARE_LOGGING_CATEGORY(logSettings)
SEGS_DECLARE_LOGGING_CATEGORY(logGUI)
SEGS_DECLARE_LOGGING_CATEGORY(logTeams)
SEGS_DECLARE_LOGGING_CATEGORY(logDB)
SEGS_DECLARE_LOGGING_CATEGORY(logInput)
SEGS_DECLARE_LOGGING_CATEGORY(logOrientation)
SEGS_DECLARE_LOGGING_CATEGORY(logChat)
SEGS_DECLARE_LOGGING_CATEGORY(logInfoMsg)
SEGS_DECLARE_LOGGING_CATEGORY(logEmotes)
SEGS_DECLARE_LOGGING_CATEGORY(logTarget)
SEGS_DECLARE_LOGGING_CATEGORY(logSpawn)
SEGS_DECLARE_LOGGING_CATEGORY(logMapEvents)
SEGS_DECLARE_LOGGING_CATEGORY(logSlashCommand)

void    setLoggingFilter();
void    toggleLogging(QString &category);
void    dumpLogging();
