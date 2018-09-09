/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Events/MessageChannels.h"
#include "Events/FloatingInfoStyles.h"
#include "Events/ClientStates.h"
#include "glm/vec3.hpp"
#include <QString>
#include <cstdint>

class QString;
class Entity;
class Character;
struct PlayerData;
struct EntityData;
struct Friend;
struct FriendsList;
struct MapClientSession;
struct CharacterPowerSet;
struct CharacterPower;
class GameDataStore;


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
bool        isEntityOnMissionMap(const EntityData &ed);

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

Entity * getEntity(MapClientSession *src, const QString &name);
Entity * getEntity(MapClientSession *src, uint32_t idx);
Entity * getEntityByDBID(class MapInstance *mi,uint32_t idx);
void    sendServerMOTD(MapClientSession *tgt);
void    on_awaiting_dead_no_gurney_test(MapClientSession &session);



// Toggles
void    toggleTeamBuffs(PlayerData &c);


const QString &getFriendDisplayMapName(const Friend &f);
QString     getEntityDisplayMapName(const EntityData &ed);
/*
 * Looking for Group
 */
void    toggleLFG(Entity &e);


/*
 * getMapServerData Wrapper to provide access to NetStructures
 */
GameDataStore *getMapServerData();


/*
 * sendInfoMessage wrapper to provide access to NetStructures
 */
void messageOutput(MessageChannel ch, QString &msg, Entity &tgt);


/*
 * SendUpdate Wrappers to provide access to NetStructures
 */
void sendClientState(MapClientSession &ent, ClientStates client_state);
void showMapXferList(MapClientSession &ent, bool has_location, glm::vec3 &location, QString &name);
void sendFloatingInfo(MapClientSession &tgt, QString &msg, FloatingInfoStyle style, float delay);
void sendFloatingNumbers(MapClientSession &src, uint32_t tgt_idx, int32_t amount);
void sendLevelUp(Entity *tgt);
void sendEnhanceCombineResponse(Entity *tgt, bool success, bool destroy);
void sendChangeTitle(Entity *tgt, bool select_origin);
void sendTrayAdd(Entity *tgt, uint32_t pset_idx, uint32_t pow_idx);
void sendFriendsListUpdate(Entity *src, const FriendsList &friends_list);
void sendSidekickOffer(Entity *tgt, uint32_t src_db_id);
void sendTeamLooking(Entity *tgt);
void sendTeamOffer(Entity *src, Entity *tgt);
void sendFaceEntity(Entity *src, uint8_t tgt_idx);
void sendFaceLocation(Entity *src, glm::vec3 &location);


const QString &getGenericTitle(uint32_t val);
const QString &getOriginTitle(uint32_t val);
/*
 * sendEmail Wrappers for providing access to Email Database
 */
void sendEmailHeaders(Entity *e);
void readEmailMessage(Entity *e, const int id);


void usePower(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, uint32_t tgt_idx, uint32_t tgt_id);

void addFriend(Entity &src, Entity &tgt);
void removeFriend(Entity &src, QString friendName);
bool isFriendOnline(Entity &src, uint32_t db_id);
void findTeamMember(Entity &tgt);

/*
 * Sidekick Methods -- Sidekick system requires teaming.
 */
bool isSidekickMentor(const Entity &e);
void inviteSidekick(Entity &src, Entity &tgt);
void addSidekick(Entity &tgt, Entity &src);
void removeSidekick(Entity &src);
void leaveTeam(Entity &e);
void removeTeamMember(class Team &self, Entity *e);
