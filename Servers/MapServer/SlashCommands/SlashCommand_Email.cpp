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

#include "SlashCommand_Email.h"

#include "DataHelpers.h"
#include "GameData/Character.h"
#include "Components/Logging.h"
#include "MapInstance.h"
#include "MessageHelpers.h"
#include "Components/Settings.h"

#include <QtCore/QString>
#include <QtCore/QDebug>

using namespace SEGSEvents;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 0 Commands
void cmdHandler_EmailHeaders(const QStringList & /*params*/, MapClientSession &sess)
{
    getEmailHeaders(sess);
}

void cmdHandler_EmailRead(const QStringList &params, MapClientSession &sess)
{
    uint32_t id = params.value(0).toInt();

    readEmailMessage(sess, id);
}

void cmdHandler_EmailSend(const QStringList &params, MapClientSession &sess)
{
    if (params.size() < 3)
    {
        sendInfoMessage(MessageChannel::SERVER, "Argument count for sending email is not correct! Please send emails from the email window instead.", sess);
        return;
    }

    // params are: recipient name, email subject, email message words
    QString recipients = params.at(0);
    // recipients from email window are enclosed in \q
    recipients.replace("\\q ", ";");
    recipients.replace("\\q", "");
    QStringList recipient_list = recipients.split(";");
    // the last element will be empty if sent through email window, so remove it
    if (recipient_list.back().isEmpty())
    {
        recipient_list.pop_back();
    }

    // first, check if your own character is one of the recipients in the email
    // cannot send email to self as that will trigger /emailRead without the data in db nor EmailHandler
    // and that will segfault the server :)
    if (recipients.contains(sess.m_ent->m_char->getName()))
    {
        sendInfoMessage(MessageChannel::SERVER, "You cannot send an email to yourself!", sess);
        return;
    }

    for (const auto &recipient : recipient_list)
        // Everything after recipient and subject is the email body
        sendEmail(sess, recipient, params.at(1), params.mid(2).join(" "));
}

void cmdHandler_EmailDelete(const QStringList &params, MapClientSession &sess)
{
    uint32_t id = params.value(0).toInt();

    deleteEmailHeaders(sess, id);

    QString msg = "Email Deleted ID: " + QString::number(id);
    qDebug().noquote() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

//! @}
