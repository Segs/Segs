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

#include "SlashCommand_Movement.h"

#include "DataHelpers.h"
#include "Components/Logging.h"
#include "MapInstance.h"
#include "MapLink.h"
#include "Messages/Map/MapXferWait.h"
#include "MessageHelpers.h"
#include "Components/Settings.h"

#include <QtCore/QString>
#include <QtCore/QDebug>

using namespace SEGSEvents;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 9 Commands (GMs)
void cmdHandler_ControlsDisabled(const QStringList &/*params*/, MapClientSession &sess)
{
    toggleControlsDisabled(*sess.m_ent);

    QString msg = "Toggling controls";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_UpdateId(const QStringList &params, MapClientSession &sess)
{
    uint8_t attrib = params.value(0).toUShort();

    setUpdateID(*sess.m_ent, attrib);

    QString msg = "Setting updateID to: " + QString::number(attrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_FullUpdate(const QStringList &/*params*/, MapClientSession &sess)
{
    toggleFullUpdate(*sess.m_ent);

    QString msg = "Toggling full update";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_HasControlId(const QStringList &/*params*/, MapClientSession &sess)
{
    toggleControlId(*sess.m_ent);

    QString msg = "Toggling has control id";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_ToggleInterp(const QStringList &/*params*/, MapClientSession &sess)
{
    toggleInterp(*sess.m_ent);

    QString msg = "Toggling interpolation";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_ToggleMoveInstantly(const QStringList &/*params*/, MapClientSession &sess)
{
    toggleMoveInstantly(*sess.m_ent);

    QString msg = "Toggling instant movement";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_ToggleCollision(const QStringList &/*params*/, MapClientSession &sess)
{
    toggleCollision(*sess.m_ent);

    QString msg = "Toggling collision";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_ToggleMovementAuthority(const QStringList &/*params*/, MapClientSession &sess)
{
    toggleMovementAuthority(*sess.m_ent);

    QString msg = "Toggling server authority for movement";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_FaceEntity(const QStringList &params, MapClientSession &sess)
{
    Entity *tgt = nullptr;

    QString name = params.join(" ");

    if(params.size() < 1)
    {
        qCDebug(logSlashCommand) << "Bad invocation:" << name;
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:" + name, sess);
        return;
    }

    tgt = getEntity(&sess, name); // get Entity by name
    if(tgt == nullptr)
    {
        QString msg = QString("FaceEntity target %1 cannot be found.").arg(name);
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
        return;
    }
    sendFaceEntity(sess, tgt->m_idx);
}

void cmdHandler_FaceLocation(const QStringList &params, MapClientSession &sess)
{
    if(params.size() < 3)
    {
        qCDebug(logSlashCommand) << "Bad invocation:" << params.join(" ");
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:" + params.join(" "), sess);
        return;
    }

    glm::vec3 loc {
      params.at(0).toFloat(),
      params.at(1).toFloat(),
      params.at(2).toFloat()
    };

    sendFaceLocation(sess, loc);
}

void cmdHandler_MoveZone(const QStringList &params, MapClientSession &sess)
{
    uint32_t map_idx = params.value(0).toUInt();
    if(map_idx == getMapIndex(sess.m_current_map->name()))
        map_idx = (map_idx + 1) % 39;   // To prevent crashing if trying to access the map you're on.
    MapXferData map_data = MapXferData();
    map_data.m_target_map_name = getMapName(map_idx);
    sess.link()->putq(new MapXferWait(getMapPath(map_idx)));

    HandlerLocator::getMap_Handler(sess.is_connected_to_game_server_id)
        ->putq(new ClientMapXferMessage({sess.link()->session_token(), map_data}, 0));
}

void cmdHandler_ToggleInputLog(const QStringList &params, MapClientSession &sess)
{
    QString name = sess.m_ent->name();
    if(!params.isEmpty())
        name = params.join(" ");

    // getEntityByNameOrTarget will always return a valid Entity, or self
    Entity* target = getEntityByNameOrTarget(sess, name);
    target->m_input_state.m_debug = !target->m_input_state.m_debug;
}

void cmdHandler_ToggleMovementLog(const QStringList &params, MapClientSession &sess)
{
    QString name = sess.m_ent->name();
    if(!params.isEmpty())
        name = params.join(" ");

    // getEntityByNameOrTarget will always return a valid Entity, or self
    Entity* target = getEntityByNameOrTarget(sess, name);
    target->m_motion_state.m_debug = !target->m_motion_state.m_debug;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 2 Commands
void cmdHandler_MoveTo(const QStringList &params, MapClientSession &sess)
{
    if(params.size() < 3)
    {
        QString errormsg = "Bad invocation. /moveto expects 3 parameters e.g. '/moveto x y z'. Received:" + params.join(" ");
        qCDebug(logSlashCommand) << errormsg;
        sendInfoMessage(MessageChannel::USER_ERROR, errormsg, sess);
        return;
    }

    glm::vec3 new_pos {
      params.at(0).toFloat(),
      params.at(1).toFloat(),
      params.at(2).toFloat()
    };
    forcePosition(*sess.m_ent, new_pos);
    sendInfoMessage(MessageChannel::DEBUG_INFO, QString("New position set"), sess);
}

void cmdHandler_Fly(const QStringList &/*params*/, MapClientSession &sess)
{
    toggleFlying(*sess.m_ent);

    QString msg = "Toggling flight";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_Jumppack(const QStringList &/*params*/, MapClientSession &sess)
{
    toggleJumppack(*sess.m_ent);

    QString msg = "Toggling jumppack";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_Teleport(const QStringList &params, MapClientSession &sess)
{
    QString msg = "Teleport format is '/teleport {target_name}'";
    QString name = sess.m_ent->name();
    if(!params.isEmpty())
        name = params.join(" ");

    // getEntityByNameOrTarget will always return a valid Entity, or self
    Entity *tgt = getEntityByNameOrTarget(sess, name);
    name = tgt->name(); // make sure we have the final name
    glm::vec3 new_pos = tgt->m_entity_data.m_pos;

    sendFaceLocation(sess, new_pos);
    forcePosition(*sess.m_ent, new_pos);

    msg = QString("Teleporting to target %1.").arg(name);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 1 Commands
void cmdHandler_Stuck(const QStringList &params, MapClientSession &sess)
{
    // TODO: Implement true move-to-safe-location-nearby logic
    //forcePosition(*sess.m_ent, sess.m_current_map->closest_safe_location(sess.m_ent->m_entity_data.m_pos));
    sess.m_current_map->setPlayerSpawn(*sess.m_ent);

    QString msg = QString("Resetting location to default spawn <%1, %2, %3>")
            .arg(sess.m_ent->m_entity_data.m_pos.x, 0, 'f', 1)
            .arg(sess.m_ent->m_entity_data.m_pos.y, 0, 'f', 1)
            .arg(sess.m_ent->m_entity_data.m_pos.z, 0, 'f', 1);

    qCDebug(logSlashCommand) << params.join(" ") << ":" << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, sess);
}

void cmdHandler_SetSpawnLocation(const QStringList &params, MapClientSession &sess)
{
    if(params.size() == 0)
    {
        // No SpawnLocation given, bail.
        return;
    }
    const QString spawnLocation = params.join(" ");
    sess.m_current_map->setSpawnLocation(*sess.m_ent, spawnLocation);
}

void cmdHandler_MapXferList(const QStringList &/*params*/, MapClientSession &sess)
{
    showMapMenu(sess);
}

//! @}
