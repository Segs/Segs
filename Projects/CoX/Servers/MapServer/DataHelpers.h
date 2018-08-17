/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Events/MessageChannels.h"
#include "glm/vec3.hpp"
#include <stdint.h>
#include <QString>
class QString;
class Entity;
class Character;
struct PlayerData;
struct EntityData;

struct Friend;
struct FriendsList;
struct MapClientSession;

/*
 * Entity Methods
 */
// Getters
uint32_t    getIdx(const Entity &e);
uint32_t    getDbId(const Entity &e);
uint32_t    getAccessLevel(const Entity &e);
uint32_t    getTargetIdx(const Entity &e);
uint32_t    getAssistTargetIdx(const Entity &e);
glm::vec3   getSpeed(const Entity &e);
float       getBackupSpd(const Entity &e);
float       getJumpHeight(const Entity &e);
uint8_t     getUpdateId(const Entity &e);

// Setters
void    setDbId(Entity &e, uint8_t val);
void    setMapName(Entity &e, const QString &val);
void    setMapIdx(Entity &e, uint32_t val);
void    setSpeed(Entity &e, float v1, float v2, float v3);
void    setBackupSpd(Entity &e, float val);
void    setJumpHeight(Entity &e, float val);
void    setUpdateID(Entity &e, uint8_t val);
void    setTeamID(Entity &e, uint8_t team_id);
void    setSuperGroup(Entity &e, int sg_id = 0, QString sg_name = "", uint32_t sg_rank = 3);
void    setTarget(Entity &e, uint32_t target_idx);
void    setAssistTarget(Entity &e);

// For live debugging
void    setu1(Entity &e, int val);

// Toggles
void    toggleFlying(Entity &e);
void    toggleFalling(Entity &e);
void    toggleJumping(Entity &e);
void    toggleSliding(Entity &e);
void    toggleStunned(Entity &e);
void    toggleJumppack(Entity &e);
void    toggleControlsDisabled(Entity &e);
void    toggleFullUpdate(Entity &e);
void    toggleControlId(Entity &e);
void    toggleExtraInfo(Entity &e);
void    toggleMoveInstantly(Entity &e);

// Misc Methods
void    charUpdateDB(Entity *e);
void    charUpdateGUI(Entity *e);
int     getEntityOriginIndex(bool is_player,const QString &origin_name);
int     getEntityClassIndex(bool is_player, const QString &class_name);
Entity * getEntity(MapClientSession *src, const QString &name);
Entity * getEntity(MapClientSession *src, uint32_t idx);
Entity * getEntityByDBID(MapClientSession &src, uint32_t idx);
void    sendServerMOTD(MapClientSession *tgt);


// Toggles
void    toggleTeamBuffs(PlayerData &c);

const QString &getFriendDisplayMapName(const Friend &f);
QString     getMapPath(const EntityData &ed);
QString     getEntityDisplayMapName(const EntityData &ed);
/*
 * Looking for Group
 */
void    toggleLFG(Entity &e);


/*
 * sendInfoMessage wrapper to provide access to NetStructures
 */
void messageOutput(MessageChannel ch, QString &msg, Entity &tgt);


/*
 * SendUpdate Wrappers to provide access to NetStructures
 */
void sendFloatingNumbers(Entity *src, uint32_t tgt_idx, int32_t amount);
void sendFriendsListUpdate(Entity *src, const FriendsList &friends_list);
void sendSidekickOffer(Entity *tgt, uint32_t src_db_id);
void sendTeamLooking(Entity *tgt);
void sendTeamOffer(Entity *src, Entity *tgt);

/*
 * sendEmail Wrappers for providing access to Email Database
 */
void sendEmailHeaders(Entity *e);
void readEmailMessage(Entity *e, const int id);
