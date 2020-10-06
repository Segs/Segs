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

#include "SlashCommand_Settings.h"

#include "DataHelpers.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/gui_definitions.h"
#include "Components/Logging.h"
#include "MapInstance.h"
#include "MessageHelpers.h"
#include "Components/Settings.h"

#include <QtCore/QString>
#include <QtCore/QDebug>

using namespace SEGSEvents;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 9 Commands
void cmdHandler_SettingsDump(const QStringList &/*params*/, MapClientSession &sess)
{
    QString msg = "Sending settings config dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    settingsDump(); // Send settings dump
}

void cmdHandler_GUIDebug(const QStringList &/*params*/, MapClientSession &sess)
{
    QString msg = "Sending GUISettings dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    sess.m_ent->m_player->m_gui.guiDump(); // Send GUISettings dump
}

void cmdHandler_SetWindowVisibility(const QStringList &params, MapClientSession &sess)
{
    uint32_t idx = params.value(0).toUInt();
    WindowVisibility val = (WindowVisibility)params.value(1).toInt();

    QString msg = "Toggling " + QString::number(idx) +  " GUIWindow visibility: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    sess.m_ent->m_player->m_gui.m_wnds.at(idx).setWindowVisibility(val); // Set WindowVisibility
    sess.m_ent->m_player->m_gui.m_wnds.at(idx).guiWindowDump(); // for debugging
}

void cmdHandler_KeybindDebug(const QStringList &/*params*/, MapClientSession &sess)
{
    QString msg = "Sending Keybinds dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    sess.m_ent->m_player->m_keybinds.keybindsDump(); // Send GUISettings dump
}

void cmdHandler_ToggleLogging(const QStringList &params, MapClientSession &sess)
{
    QString msg = "Toggle logging of categories: " + params.join(" ");
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    for (auto category : params)
        toggleLogging(category); // Toggle each category listed
}

//! @}
