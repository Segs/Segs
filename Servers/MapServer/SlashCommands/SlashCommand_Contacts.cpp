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

#include "SlashCommand_Contacts.h"

#include "DataHelpers.h"
#include "Components/Logging.h"
#include "MapInstance.h"
#include "MessageHelpers.h"
#include "Components/Settings.h"

#include <QtCore/QString>
#include <QtCore/QDebug>

using namespace SEGSEvents;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 9 Commands
void cmdHandler_SendContactDialog(const QStringList &params, MapClientSession &sess)
{
    QString content = params.join(" ");
    std::vector<ContactEntry> active_contacts;
    // TODO: Derive number to send from parameters
    int num_contacts_to_send = 4;

    for(int i = 0; i < num_contacts_to_send; ++i)
    {
        ContactEntry con;
        con.m_response_text = QString("Response #%1").arg(i);
        con.m_link = i; // a reference to contactLinkHash?
        active_contacts.push_back(con);
    }

    sendContactDialog(sess, content, active_contacts);
}

void cmdHandler_SendContactDialogYesNoOk(const QStringList &params, MapClientSession &sess)
{
    if(params.size() < 2)
    {
        qCDebug(logSlashCommand) << "Bad invocation:" << params.join(" ") << " requires at least two arguments";
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:" + params.join(" ") + " requires at least two arguments", sess);
        return;
    }

    bool ok = true;
    bool has_yesno = params.at(0).toInt(&ok);
    // Combine params after int and use those as dialog content
    QString content = params.mid(1).join(" ");

    if(!ok)
    {
        qCDebug(logSlashCommand) << "First argument must be boolean value;" << content;
        sendInfoMessage(MessageChannel::USER_ERROR, "First argument must be boolean value;" + content, sess);
        return;
    }

    sendContactDialogYesNoOk(sess, content, has_yesno);
}

void cmdHandler_ContactStatusList(const QStringList &/*params*/, MapClientSession &sess)
{
    Contact startingContact;
    startingContact.setName("Officer Flint"); // "OfficerFlint
    startingContact.m_current_standing = 0;
    startingContact.m_notify_player = true;
    startingContact.m_task_index = 1;
    startingContact.m_npc_id = 1939; // Npc Id
    startingContact.m_has_location = true;
    startingContact.m_task_index = 0;
    startingContact.m_location_description = "Outbreak";
    startingContact.m_location.location.x = -62.0;
    startingContact.m_location.location.y = 0.0;
    startingContact.m_location.location.z = 182.0;
    startingContact.m_location.m_location_name = "Outbreak Starting";
    startingContact.m_location.m_location_map_name = "City_00_01"; //folder name?
    startingContact.m_confidant_threshold = 3;
    startingContact.m_friend_threshold = 2;
    startingContact.m_complete_threshold = 4;
    startingContact.m_can_use_cell = false;

    updateContactStatusList(sess, startingContact);
    QString msg = "Sending OfficerFlint to contactList";
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_AddTestTask(const QStringList &/*params*/, MapClientSession &sess)
{
    Task tk;
    tk.m_db_id = 1;
    tk.setDescription("Task Description Goes Here");
    tk.setOwner("OfficerFlint");
    tk.setState("In Progress");
    tk.setDetail("Task detail goes here");
    tk.m_is_complete = false;
    tk.m_in_progress_maybe = true;
    tk.m_is_abandoned = false;
    tk.m_has_location = false;
    tk.m_detail_invalid = true; // aka "needs update"
    tk.m_board_train = false;
    tk.m_location.location = glm::vec3(-83.0, 0.0, 1334.0);
    tk.m_location.setLocationMapName("Outbreak");
    tk.m_location.setLocationName("Outbreak");
    tk.m_finish_time = 0;
    tk.m_unknown_1 = 0;
    tk.m_unknown_2 = 0;
    tk.m_task_idx = 0;

    sendUpdateTaskStatusList(sess, tk);
    QString msg = "Sending Test Task to client";
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_OpenStore(const QStringList &/*params*/, MapClientSession &sess)
{
    qCDebug(logSlashCommand) << "OpenStore...";
    openStore(sess, 0); // Default entity_idx as it doesn't change anything currently
}

//! @}
