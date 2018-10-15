/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Events/MessageChannels.h"
#include "glm/vec3.hpp"
#include <vector>

class QString;
class Entity;
class Character;
struct Friend;
struct FriendsList;
struct MapClientSession;
struct CharacterPowerSet;
struct CharacterPower;
struct PowerStance;
class GameDataStore;
class TradeMember;
struct ContactEntry;
enum FloatingInfoStyle : int;
enum class ClientStates : uint8_t;

/*
 * This file is intended to hold helper functions for methods
 * requiring access to MapClientSession or MapInstance
 */

Entity * getEntity(MapClientSession *src, const QString &name);
Entity * getEntity(MapClientSession *src, uint32_t idx);
Entity * getEntityByDBID(class MapInstance *mi,uint32_t idx);
void    sendServerMOTD(MapClientSession *sess);
bool    isFriendOnline(MapClientSession &sess, uint32_t db_id);
void    setInterpolationSettings(MapClientSession *sess, const bool active, const uint8_t level, const uint8_t bits);


/*
 * sendEmail Wrappers for providing access to Email Database
 */
void sendEmailHeaders(MapClientSession& sess);
void readEmailMessage(MapClientSession& sess, const uint32_t email_id);
void sendEmail(MapClientSession& sess, QString recipient_name, QString subject, QString message);
void deleteEmailHeaders(MapClientSession& sess, const uint32_t email_id);


/*
 * sendInfoMessage wrapper to provide access to NetStructures
 */
void messageOutput(MessageChannel ch, const QString &msg, Entity &tgt);


/*
 * SendUpdate Wrappers
 */
void sendInfoMessage(MessageChannel ch, QString msg, MapClientSession &tgt);
void sendTimeStateLog(MapClientSession &sess, uint32_t control_log);
void sendTimeUpdate(MapClientSession &sess, int32_t sec_since_jan_1_2000);
void sendClientState(MapClientSession &sess, ClientStates client_state);
void showMapXferList(MapClientSession &sess, bool has_location, glm::vec3 &location, QString &name);
void sendFloatingInfo(MapClientSession &sess, QString &msg, FloatingInfoStyle style, float delay);
void sendFloatingNumbers(MapClientSession &sess, uint32_t tgt_idx, int32_t amount);
void sendLevelUp(MapClientSession &sess);
void sendEnhanceCombineResponse(MapClientSession &sess, bool success, bool destroy);
void sendChangeTitle(MapClientSession &sess, bool select_origin);
void sendTrayAdd(MapClientSession &sess, uint32_t pset_idx, uint32_t pow_idx);
void sendFriendsListUpdate(MapClientSession &sess, const FriendsList &friends_list);
void sendSidekickOffer(MapClientSession &sess, uint32_t src_db_id);
void sendTeamLooking(MapClientSession &sess);
void sendTeamOffer(MapClientSession &src, MapClientSession &tgt);
void sendFaceEntity(MapClientSession &src, int32_t tgt_idx);
void sendFaceLocation(MapClientSession &sess, glm::vec3 &loc);
void sendDoorMessage(MapClientSession &sess, uint32_t delay_status, QString &msg);
void sendBrowser(MapClientSession &sess, QString &content);
void sendTradeOffer(MapClientSession &tgt, const QString &name);
void sendTradeInit(MapClientSession &src, MapClientSession &tgt);
void sendTradeCancel(MapClientSession &sess, const QString &msg);
void sendTradeUpdate(MapClientSession &src, MapClientSession &tgt, const TradeMember& trade_src, const TradeMember& trade_tgt);
void sendTradeSuccess(MapClientSession &src, MapClientSession &tgt);
void sendContactDialog(MapClientSession &sess, QString msg_body, std::vector<ContactEntry> active_contacts);
void sendContactDialogYesNoOk(MapClientSession &sess, QString msg_body, bool has_yesno);
void sendContactDialogClose(MapClientSession &sess);
void sendWaypoint(MapClientSession &sess, int point_idx, glm::vec3 location);
void sendStance(MapClientSession &sess, PowerStance stance);
void sendDeadNoGurney(MapClientSession &sess);

/*
 * usePower and increaseLevel here to provide access to
 * both Entity and sendInfoMessage
 */
void usePower(Entity &ent, uint32_t pset_idx, uint32_t pow_idx, int32_t tgt_idx, int32_t tgt_id);
void increaseLevel(Entity &ent);

/*
 * Lua Functions
 */
void addNpc(MapClientSession &cl, const char* name, glm::vec3 *loc, int variation);
void giveEnhancement(MapClientSession *cl, const char* name, int level);
void giveDebt(MapClientSession *cl, int debt);
void giveEnd(MapClientSession *cl, float end);
void giveHp(MapClientSession *cl, float hp);
void giveInf(MapClientSession *cl, int inf);
void giveInsp(MapClientSession *cl, const char *value);
void giveXp(MapClientSession *cl, int xp);
