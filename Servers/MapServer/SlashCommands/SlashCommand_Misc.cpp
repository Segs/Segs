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

#include "SlashCommand_Misc.h"

#include "DataHelpers.h"
#include "GameData/CharacterHelpers.h"
#include "Components/Logging.h"
#include "MapInstance.h"
#include "MessageHelpers.h"
#include "Components/Settings.h"

#include <QtCore/QString>
#include <QtCore/QDebug>

using namespace SEGSEvents;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 2 Commands
uint cmdHandler_AddNPC(const QStringList &params, MapClientSession &sess)
{
    if(params.size() < 1)
    {
        qCDebug(logSlashCommand) << "Bad invocation:" << params.join(" ");
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:" + params.join(" "), sess);
        return 0;
    }
    QString name = params.at(0);
    // Variation may not be supplied, so default to 0
    int variation = params.value(1).toInt();

    glm::vec3 offset = glm::vec3 {2,0,1};
    glm::vec3 gm_loc = sess.m_ent->m_entity_data.m_pos + offset;
    return addNpc(sess, name, gm_loc, variation, name);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 1 Commands
void cmdHandler_WhoAll(const QStringList &/*params*/, MapClientSession &sess)
{
    MapInstance *     mi  = sess.m_current_map;

    QString msg = "Players on this map:\n";

    for (MapClientSession *cl : mi->m_session_store)
    {
        Character &c(*cl->m_ent->m_char);
        QString    name      = cl->m_ent->name();
        QString    lvl       = QString::number(getLevel(c)+1);         //+1 as the server stores these values
        QString    clvl      = QString::number(getCombatLevel(c)+1);  //with a 0 index, issue #831
        QString    origin    = getOrigin(c);
        QString    archetype = QString(getClass(c)).remove("Class_");

        // Format: character_name "lvl" level "clvl" combat_level origin archetype
        msg += QString("%1 lvl %2 clvl %3 %4 %5\n").arg(name,lvl,clvl,origin,archetype);
    }

    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, sess);
}

void cmdHandler_MOTD(const QStringList &/*params*/, MapClientSession &sess)
{
    sendServerMOTD(&sess);
    QString msg = "Opening Server MOTD";
    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, sess);
}

void cmdHandler_Tailor(const QStringList &/*params*/, MapClientSession &sess)
{
    sendTailorOpen(sess);
}

void cmdHandler_CostumeChange(const QStringList &params, MapClientSession &sess)
{
    uint32_t costume_idx = params.value(0).toUInt();

    setCurrentCostumeIdx(*sess.m_ent->m_char, costume_idx);

    QString msg = "Changing costume to: " + QString::number(costume_idx);
    qCDebug(logTailor) << msg;
}

void cmdHandler_Train(const QStringList &/*params*/, MapClientSession &sess)
{
    playerTrain(sess);
}

void cmdHandler_Kiosk(const QStringList &/*params*/, MapClientSession &sess)
{
    sendKiosk(sess);
}


//! @}
