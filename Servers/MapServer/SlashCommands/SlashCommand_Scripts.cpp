/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup SlashCommands Projects/CoX/Servers/MapServer/SlashCommands
 * @{
 */

#include "SlashCommand_Scripts.h"

#include "ScriptingEngine/ScriptingEngine.h"
#include "DataHelpers.h"
#include "Components/Logging.h"
#include "MapInstance.h"
#include "Messages/Map/StandardDialogCmd.h"
#include "MessageHelpers.h"

#include <QtCore/QString>
#include <QtCore/QDebug>

using namespace SEGSEvents;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 9 Commands
void cmdHandler_Script(const QStringList &params, MapClientSession &sess)
{
    QString code = params.join(" ");
    sess.m_current_map->m_scripting_interface->runScript(&sess, code, "user provided script");
}

void cmdHandler_SmileX(const QStringList &params, MapClientSession &sess)
{
    QString fileName("scripts/" + params.join(" "));
    if(!fileName.endsWith(".smlx"))
            fileName.append(".smlx");
    QFile file(fileName);
    if(file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString contents(file.readAll());
        sess.addCommandToSendNextUpdate(std::make_unique<StandardDialogCmd>(contents));
    }
    else {
        QString errormsg = "Failed to load smilex file. \'" + file.fileName() + "\' not found.";
        qCDebug(logSlashCommand) << errormsg;
        sendInfoMessage(MessageChannel::ADMIN, errormsg, sess);
    }
}

void cmdHandler_ReloadScripts(const QStringList &/*params*/, MapClientSession &sess)
{
    qCDebug(logSlashCommand) << "Reloading all Lua scripts in" << sess.m_current_map->name();

    // Reset script engine
    sess.m_current_map->m_scripting_interface.reset(new ScriptingEngine);
    sess.m_current_map->m_scripting_interface->setIncludeDir(sess.m_current_map->name());
    sess.m_current_map->m_scripting_interface->registerTypes();

    // load all scripts again
    // TODO: this will regenerate any NPCs (luabot) that exist
    sess.m_current_map->load_map_lua();
}

//! @}
