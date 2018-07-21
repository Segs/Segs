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
void    setu1(Entity &e, float val);

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
Entity * getEntityByDBID(MapClientSession *src, uint32_t idx);
void    sendServerMOTD(MapClientSession *tgt);
void    positionTest(Entity *e);
void    setInterpolationSettings(Entity *e, const bool active, const uint8_t level, const uint8_t bits);

/*
 * Character Methods
 */
// Getters
uint32_t            getLevel(const Character &c);
uint32_t            getCombatLevel(const Character &c);
float getHP(const Character &c);
float getEnd(const Character &c);
uint64_t            getLastCostumeId(const Character &c);
const QString &     getOrigin(const Character &c);
const QString &     getClass(const Character &c);
const QString &     getMapName(const Character &c);
uint32_t            getXP(const Character &c);
uint32_t            getDebt(const Character &c);
uint32_t            getPatrolXP(const Character &c);
const QString &     getGenericTitle(const Character &c);
const QString &     getOriginTitle(const Character &c);
const QString &     getSpecialTitle(const Character &c);

uint32_t            getInf(const Character &c);
const QString &     getDescription(const Character &c);
const QString &     getBattleCry(const Character &c);
const QString &     getAlignment(const Character &c);

// Setters
void    setLevel(Character &c, uint32_t val);
void    setCombatLevel(Character &c, uint32_t val);
void    setHP(Character &c, float val);
void    setEnd(Character &c, float val);
void    setLastCostumeId(Character &c, uint64_t val);
void    setMapName(Character &c, const QString &val);
void    setXP(Character &c, uint32_t val);
void    setDebt(Character &c, uint32_t val);
void    setTitles(Character &c, bool prefix = false, QString generic = "", QString origin = "", QString special = "");
void    setInf(Character &c, uint32_t val);
void    setDescription(Character &c, QString val);
void    setBattleCry(Character &c, QString val);

// Toggles
void    toggleAFK(Character &c, const QString &msg = "");
void    toggleTeamBuffs(PlayerData &c);


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
void sendFriendsListUpdate(Entity *src, FriendsList *friends_list);
void sendSidekickOffer(Entity *tgt, uint32_t src_db_id);
void sendTeamLooking(Entity *tgt);
void sendTeamOffer(Entity *src, Entity *tgt);


/*
 * sendEmail Wrappers for providing access to Email Database
 */
void sendEmailHeaders(Entity *e);
void readEmailMessage(Entity *e, const int id);
