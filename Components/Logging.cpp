/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup Components
 * @{
 */

#include "Logging.h"
#include "Settings.h"

#define SEGS_LOGGING_CATEGORY(name, string) \
    QLoggingCategory &name() \
    { \
        static QLoggingCategory category(string); \
        return category; \
    }

SEGS_LOGGING_CATEGORY(logLogging,      "log.logging")
SEGS_LOGGING_CATEGORY(logKeybinds,     "log.keybinds")
SEGS_LOGGING_CATEGORY(logSettings,     "log.settings")
SEGS_LOGGING_CATEGORY(logGUI,          "log.gui")
SEGS_LOGGING_CATEGORY(logTeams,        "log.teams")
SEGS_LOGGING_CATEGORY(logDB,           "log.db")
SEGS_LOGGING_CATEGORY(logInput,        "log.input")
SEGS_LOGGING_CATEGORY(logOrientation,  "log.orientation")
SEGS_LOGGING_CATEGORY(logPosition,     "log.position")
SEGS_LOGGING_CATEGORY(logChat,         "log.chat")
SEGS_LOGGING_CATEGORY(logInfoMsg,      "log.infomsg")
SEGS_LOGGING_CATEGORY(logEmotes,       "log.emotes")
SEGS_LOGGING_CATEGORY(logTarget,       "log.target")
SEGS_LOGGING_CATEGORY(logSpawn,        "log.spawn")
SEGS_LOGGING_CATEGORY(logMapEvents,    "log.mapevents")
SEGS_LOGGING_CATEGORY(logSlashCommand, "log.slashcommand")
SEGS_LOGGING_CATEGORY(logDescription,  "log.description")
SEGS_LOGGING_CATEGORY(logFriends,      "log.friends")
SEGS_LOGGING_CATEGORY(logMiniMap,      "log.minimap")
SEGS_LOGGING_CATEGORY(logLFG,          "log.lfg")
SEGS_LOGGING_CATEGORY(logNPCs,         "log.npcs")
SEGS_LOGGING_CATEGORY(logAnimations,         "log.animations")

void setLoggingFilter()
{
    QSettings config(Settings::getSettingsPath(),QSettings::IniFormat,nullptr);

    config.beginGroup("Logging");
    QString filter_rules = config.value("log_generic","*.debug=true\nqt.*.debug=false").toString();
    filter_rules += "\nlog.logging="        + config.value("log_logging","false").toString();
    filter_rules += "\nlog.keybinds="       + config.value("log_keybinds","false").toString();
    filter_rules += "\nlog.settings="       + config.value("log_settings","false").toString();
    filter_rules += "\nlog.gui="            + config.value("log_gui","false").toString();
    filter_rules += "\nlog.teams="          + config.value("log_teams","false").toString();
    filter_rules += "\nlog.db="             + config.value("log_db","false").toString();
    filter_rules += "\nlog.input="          + config.value("log_input","false").toString();
    filter_rules += "\nlog.position="       + config.value("log_position","false").toString();
    filter_rules += "\nlog.orientation="    + config.value("log_orientation","false").toString();
    filter_rules += "\nlog.chat="           + config.value("log_chat","false").toString();
    filter_rules += "\nlog.infomsg="        + config.value("log_infomsg","false").toString();
    filter_rules += "\nlog.emotes="         + config.value("log_emotes","true").toString();
    filter_rules += "\nlog.target="         + config.value("log_target","false").toString();
    filter_rules += "\nlog.spawn="          + config.value("log_spawn","false").toString();
    filter_rules += "\nlog.mapevents="      + config.value("log_mapevents","true").toString();
    filter_rules += "\nlog.slashcommand="   + config.value("log_slashcommand","true").toString();
    filter_rules += "\nlog.description="    + config.value("log_description","false").toString();
    filter_rules += "\nlog.friends="        + config.value("log_friends","false").toString();
    filter_rules += "\nlog.minimap="        + config.value("log_minimap","false").toString();
    filter_rules += "\nlog.lfg="            + config.value("log_lfg","false").toString();
    filter_rules += "\nlog.npcs="           + config.value("log_npcs","false").toString();
    filter_rules += "\nlog.animations="     + config.value("log_animations","false").toString();
    config.endGroup(); // Logging

    QLoggingCategory::setFilterRules(filter_rules);

    qCDebug(logLogging) << "Logging FilterRules:" << filter_rules; // so meta
}

void toggleLogging(QString &category)
{
    if(category.isEmpty())
        return;

    QLoggingCategory *cat = nullptr;

    if(category.contains("logging",Qt::CaseInsensitive))
        cat = &logLogging();
    else if(category.contains("keybinds",Qt::CaseInsensitive))
        cat = &logKeybinds();
    else if(category.contains("settings",Qt::CaseInsensitive))
        cat = &logSettings();
    else if(category.contains("gui",Qt::CaseInsensitive))
        cat = &logGUI();
    else if(category.contains("teams",Qt::CaseInsensitive))
        cat = &logTeams();
    else if(category.contains("db",Qt::CaseInsensitive))
        cat = &logDB();
    else if(category.contains("input",Qt::CaseInsensitive))
        cat = &logInput();
    else if(category.contains("position",Qt::CaseInsensitive))
        cat = &logPosition();
    else if(category.contains("orentation",Qt::CaseInsensitive))
        cat = &logOrientation();
    else if(category.contains("chat",Qt::CaseInsensitive))
        cat = &logChat();
    else if(category.contains("infomsg",Qt::CaseInsensitive))
        cat = &logInfoMsg();
    else if(category.contains("emotes",Qt::CaseInsensitive))
        cat = &logEmotes();
    else if(category.contains("target",Qt::CaseInsensitive))
        cat = &logTarget();
    else if(category.contains("spawn",Qt::CaseInsensitive))
        cat = &logSpawn();
    else if(category.contains("mapevents",Qt::CaseInsensitive))
        cat = &logMapEvents();
    else if(category.contains("slashcommand",Qt::CaseInsensitive))
        cat = &logSlashCommand();
    else if(category.contains("description",Qt::CaseInsensitive))
        cat = &logDescription();
    else if(category.contains("friends",Qt::CaseInsensitive))
        cat = &logFriends();
    else if(category.contains("minimap",Qt::CaseInsensitive))
        cat = &logMiniMap();
    else if(category.contains("lfg",Qt::CaseInsensitive))
        cat = &logLFG();
    else if(category.contains("npcs",Qt::CaseInsensitive))
        cat = &logNPCs();
    else if(category.contains("animations",Qt::CaseInsensitive))
        cat = &logAnimations();
    else
        return;

    cat->setEnabled(QtDebugMsg, !cat->isDebugEnabled());
    cat->setEnabled(QtInfoMsg, !cat->isInfoEnabled());
    cat->setEnabled(QtWarningMsg, !cat->isWarningEnabled());
    cat->setEnabled(QtCriticalMsg, !cat->isCriticalEnabled());

    dumpLogging();
}

void dumpLogging()
{
    QString output = "Current Logging Categories:";
    output += "\n\t logging: "      + QString::number(logLogging().isDebugEnabled());
    output += "\n\t keybinds: "     + QString::number(logKeybinds().isDebugEnabled());
    output += "\n\t settings: "     + QString::number(logSettings().isDebugEnabled());
    output += "\n\t gui: "          + QString::number(logGUI().isDebugEnabled());
    output += "\n\t teams: "        + QString::number(logTeams().isDebugEnabled());
    output += "\n\t db: "           + QString::number(logDB().isDebugEnabled());
    output += "\n\t input: "        + QString::number(logInput().isDebugEnabled());
    output += "\n\t position: "     + QString::number(logLFG().isDebugEnabled());
    output += "\n\t orientation: "  + QString::number(logOrientation().isDebugEnabled());
    output += "\n\t chat: "         + QString::number(logChat().isDebugEnabled());
    output += "\n\t infomsg: "      + QString::number(logInfoMsg().isDebugEnabled());
    output += "\n\t emotes: "       + QString::number(logEmotes().isDebugEnabled());
    output += "\n\t target: "       + QString::number(logTarget().isDebugEnabled());
    output += "\n\t spawn: "        + QString::number(logSpawn().isDebugEnabled());
    output += "\n\t mapevents: "    + QString::number(logMapEvents().isDebugEnabled());
    output += "\n\t slashcommand: " + QString::number(logSlashCommand().isDebugEnabled());
    output += "\n\t description: "  + QString::number(logDescription().isDebugEnabled());
    output += "\n\t friends: "      + QString::number(logFriends().isDebugEnabled());
    output += "\n\t minimap: "      + QString::number(logMiniMap().isDebugEnabled());
    output += "\n\t lfg: "          + QString::number(logLFG().isDebugEnabled());
    output += "\n\t npcs: "         + QString::number(logNPCs().isDebugEnabled());
    output += "\n\t animations: "   + QString::number(logAnimations().isDebugEnabled());

    qDebug().noquote() << output;
}

//! @}
