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

#include "SlashCommand_SuperGroup.h"

#include "GameData/Character.h"
#include "GameData/EntityHelpers.h"
#include "Components/Logging.h"
#include "MapInstance.h"
#include "MessageHelpers.h"

#include <QtCore/QString>
#include <QtCore/QDebug>

using namespace SEGSEvents;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 9 Commands (GMs)
void cmdHandler_SetSuperGroup(const QStringList &params, MapClientSession &sess)
{
    int sg_id       = params.value(0).toInt();
    QString sg_name = params.value(1);
    int sg_rank     = params.value(2).toInt();

    setSuperGroup(*sess.m_ent, sg_id, sg_name, sg_rank);

    QString msg = QString("Set SuperGroup:  id: %1  name: %2  rank: %3").arg(QString::number(sg_id), sg_name, QString::number(sg_rank));
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

//! @}
