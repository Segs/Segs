/*
 * Super Entity Game Server
 * http://github.com/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "Logging.h"
#include "Settings.h"

#define SEGS_LOGGING_CATEGORY(name, string) \
    QLoggingCategory &name() \
    { \
        static QLoggingCategory category(string); \
        return category; \
    }

SEGS_LOGGING_CATEGORY(logKeybinds,     "log.keybinds")
SEGS_LOGGING_CATEGORY(logSettings,     "log.settings")
SEGS_LOGGING_CATEGORY(logGUI,          "log.gui")
SEGS_LOGGING_CATEGORY(logTeams,        "log.teams")
SEGS_LOGGING_CATEGORY(logDB,           "log.db")
SEGS_LOGGING_CATEGORY(logInput,        "log.input")
SEGS_LOGGING_CATEGORY(logOrientation,  "log.orientation")
SEGS_LOGGING_CATEGORY(logChat,         "log.chat")
SEGS_LOGGING_CATEGORY(logInfoMsg,      "log.infomsg")
SEGS_LOGGING_CATEGORY(logEmotes,       "log.emotes")
SEGS_LOGGING_CATEGORY(logTarget,       "log.target")
SEGS_LOGGING_CATEGORY(logSpawn,        "log.spawn")
SEGS_LOGGING_CATEGORY(logMapEvents,    "log.mapevents")
SEGS_LOGGING_CATEGORY(logSlashCommand, "log.slashcommand")

void setLoggingFilter()
{
    QSettings *config(Settings::getSettings());

    QString filter_rules = config->value("log_generic","*.debug=true\nqt.*.debug=false").toString();
    filter_rules += "\nlog.keybinds="       + config->value("log_keybinds","false").toString();
    filter_rules += "\nlog.settings="       + config->value("log_settings","false").toString();
    filter_rules += "\nlog.gui="            + config->value("log_gui","false").toString();
    filter_rules += "\nlog.teams="          + config->value("log_teams","false").toString();
    filter_rules += "\nlog.db="             + config->value("log_db","false").toString();
    filter_rules += "\nlog.input="          + config->value("log_input","false").toString();
    filter_rules += "\nlog.orientation="    + config->value("log_orientation","false").toString();
    filter_rules += "\nlog.chat="           + config->value("log_chat","false").toString();
    filter_rules += "\nlog.infomsg="        + config->value("log_infomsg","false").toString();
    filter_rules += "\nlog.emotes="         + config->value("log_emotes","true").toString();
    filter_rules += "\nlog.target="         + config->value("log_target","false").toString();
    filter_rules += "\nlog.spawn="          + config->value("log_spawn","false").toString();
    filter_rules += "\nlog.mapevents="      + config->value("log_mapevents","true").toString();
    filter_rules += "\nlog.slashcommand="   + config->value("log_slashcommand","true").toString();

    QLoggingCategory::setFilterRules(filter_rules);
}

void toggleLogging(QString &category)
{
    if(category.isEmpty())
        return;

    QLoggingCategory *cat = nullptr;

    if(category.contains("keybinds",Qt::CaseInsensitive))
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
    output += "\n\t keybinds: "     + QString(logKeybinds().isDebugEnabled());
    output += "\n\t settings: "     + QString(logSettings().isDebugEnabled());
    output += "\n\t gui: "          + QString(logGUI().isDebugEnabled());
    output += "\n\t teams: "        + QString(logTeams().isDebugEnabled());
    output += "\n\t db: "           + QString(logDB().isDebugEnabled());
    output += "\n\t input: "        + QString(logInput().isDebugEnabled());
    output += "\n\t orientation: "  + QString(logOrientation().isDebugEnabled());
    output += "\n\t chat: "         + QString(logChat().isDebugEnabled());
    output += "\n\t infomsg: "      + QString(logInfoMsg().isDebugEnabled());
    output += "\n\t emotes: "       + QString(logEmotes().isDebugEnabled());
    output += "\n\t target: "       + QString(logTarget().isDebugEnabled());
    output += "\n\t spawn: "        + QString(logSpawn().isDebugEnabled());
    output += "\n\t mapevents: "    + QString(logMapEvents().isDebugEnabled());
    output += "\n\t slashcommand: " + QString(logSlashCommand().isDebugEnabled());

    qDebug().noquote() << output;
}
