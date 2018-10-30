/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Messages/Map/MessageChannels.h"
#include "Messages/Map/FloatingInfoStyles.h"
#include "Common/GameData/Contact.h"
#include "glm/vec3.hpp"
#include <QString>
#include <cstdint>
#include <vector>

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
class TradeMember;
struct ContactEntry;
struct Destination;
struct PowerStance;
enum class ClientStates : uint8_t;

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
Destination     getCurrentDestination(const Entity &e);
ClientStates    getStateMode(const Entity &e);

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
void    setCurrentDestination(Entity &e, int point_idx, glm::vec3 location);

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
void    toggleInterp(Entity &e);
void    toggleMoveInstantly(Entity &e);
void    toggleCollision(Entity &e);
void    toggleMovementAuthority(Entity &e);
void    toggleTeamBuffs(PlayerData &c);
void    toggleLFG(Entity &e);

// Misc Methods
void    charUpdateDB(Entity *e);

Entity * getEntity(MapClientSession *src, const QString &name);
Entity * getEntity(MapClientSession *src, uint32_t idx);
Entity * getEntityByDBID(class MapInstance *mi,uint32_t idx);
void    sendServerMOTD(MapClientSession *tgt);
void    positionTest(MapClientSession *tgt);
bool    isFriendOnline(Entity &src, uint32_t db_id);
void    setInterpolationSettings(MapClientSession *sess, const bool active, const uint8_t level, const uint8_t bits);
QString createMapMenu();


/*
 * Titles -- TODO: get titles from texts/English/titles_def
 */
const QString &getGenericTitle(uint32_t val);
const QString &getOriginTitle(uint32_t val);


/*
 * sendInfoMessage wrapper to provide access to NetStructures
 */
void messageOutput(MessageChannel ch, const QString &msg, Entity &tgt);


/*
 * SendUpdate Wrappers to provide access to NetStructures
 */
void sendTimeStateLog(MapClientSession &src, uint32_t control_log);
void sendTimeUpdate(MapClientSession &src, int32_t sec_since_jan_1_2000);
void sendClientState(MapClientSession &sess, ClientStates client_state);
void showMapXferList(MapClientSession &sess, bool has_location, glm::vec3 &location, QString &name);
void sendFloatingInfo(MapClientSession &tgt, QString &msg, FloatingInfoStyle style, float delay);
void sendFloatingNumbers(MapClientSession &src, uint32_t tgt_idx, int32_t amount);
void sendLevelUp(MapClientSession &src);
void sendEnhanceCombineResponse(Entity *tgt, bool success, bool destroy);
void sendChangeTitle(Entity *tgt, bool select_origin);
void sendTrayAdd(Entity *tgt, uint32_t pset_idx, uint32_t pow_idx);
void sendFriendsListUpdate(Entity *src, const FriendsList &friends_list);
void sendSidekickOffer(Entity *tgt, uint32_t src_db_id);
void sendTeamLooking(Entity *tgt);
void sendTeamOffer(Entity *src, Entity *tgt);
void sendFaceEntity(Entity &src, int32_t tgt_idx);
void sendFaceLocation(Entity &src, glm::vec3 &loc);
void sendDoorMessage(MapClientSession &tgt, uint32_t delay_status, QString &msg);
void sendBrowser(MapClientSession &tgt, QString &content);
void sendTradeOffer(const Entity& src, Entity& tgt);
void sendTradeInit(Entity& src, Entity& tgt);
void sendTradeCancel(Entity& ent, const QString& msg);
void sendTradeUpdate(Entity& src, Entity& tgt, const TradeMember& trade_src, const TradeMember& trade_tgt);
void sendTradeSuccess(Entity& src, Entity& tgt);
void sendContactDialog(MapClientSession &src, QString msg_body, std::vector<ContactEntry> active_contacts);
void sendContactDialogYesNoOk(MapClientSession &src, QString msg_body, bool has_yesno);
void sendContactDialogClose(MapClientSession &src);
void sendContactStatusList(MapClientSession &src);
void updateContactStatusList(MapClientSession &src, Contact contact);
void sendWaypoint(MapClientSession &src, int point_idx, glm::vec3 &location);
void sendStance(MapClientSession &src, PowerStance stance);
void sendDeadNoGurney(MapClientSession &sess);
void sendDoorAnimStart(MapClientSession &sess, glm::vec3 &entry_pos, glm::vec3 &target_pos, bool has_anims, QString &seq_state);
void sendDoorAnimExit(MapClientSession &sess, bool force_move);

const QString &getGenericTitle(uint32_t val);
const QString &getOriginTitle(uint32_t val);

/*
 * sendEmail Wrappers for providing access to Email Database
 */
void sendEmailHeaders(MapClientSession& sess);
void readEmailMessage(MapClientSession& sess, const uint32_t email_id);
void sendEmail(MapClientSession& sess, QString recipient_name, QString subject, QString message);
void deleteEmailHeaders(MapClientSession& sess, const uint32_t email_id);

/*
 * usePower exposed for future Lua support
 */
void usePower(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, int32_t tgt_idx, int32_t tgt_id);
void increaseLevel(Entity &ent);

/*
 * Team related helpers
 */
void findTeamMember(Entity &tgt);


/*
 * Lua Functions
 */
void addNpc(MapClientSession &sess, QString &name, glm::vec3 &loc, int variation);
void addNpcWithOrientation(MapClientSession &sess, QString &name, glm::vec3 &loc, int variation, glm::vec3 &ori);
void giveEnhancement(MapClientSession &sess, QString &name, int level);
void giveDebt(MapClientSession &sess, int debt);
void giveEnd(MapClientSession &sess, float end);
void giveHp(MapClientSession &sess, float hp);
void giveInf(MapClientSession &sess, int inf);
void giveInsp(MapClientSession &sess, QString &name);
void giveXp(MapClientSession &sess, int xp);
