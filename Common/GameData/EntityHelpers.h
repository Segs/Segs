/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "CommonNetStructures.h"
#include "glm/vec3.hpp"
#include <stdint.h>
#include <QString>
#include "Powers.h"

class Entity;
struct PlayerData;
struct EntityData;
struct Destination;
struct PowerStance;
struct Parse_NPC;
class GameDataStore;
enum class ClientStates : uint8_t;
enum class ReviveLevel;

enum class DbStoreFlags : uint32_t
{
    PlayerData = 1,
    Full       = ~0U,
};

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
void    resetSpeed(Entity &e);
void    setTeamID(Entity &e, uint8_t team_id);
void    setSuperGroup(Entity &e, int sg_id = 0, QString sg_name = "", uint32_t sg_rank = 3);
void    setTarget(Entity &e, uint32_t target_idx);
void    setAssistTarget(Entity &e, uint32_t target_idx);
void    setCurrentDestination(Entity &e, int point_idx, glm::vec3 location);
void    setStateMode(Entity &e, ClientStates state);
bool    validTarget(Entity &target_ent, Entity &ent, StoredEntEnum const &target);
bool    validTargets(Entity &target_ent, Entity &ent, std::vector<StoredEntEnum> const &targets);
void    modifyAttrib(Entity &e, buffset change);
void    checkMovement(Entity &e);
bool    checkPowerBlock(Entity &e);

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
void abortLogout(Entity *e);
void initializeNewPlayerEntity(Entity &e);
void initializeNewNpcEntity(const GameDataStore &data, Entity &e, const Parse_NPC *src, int idx, int variant);
void initializeNewCritterEntity(const GameDataStore &data, Entity &e, const Parse_NPC *src, int idx, int variant, int level);
void fillEntityFromNewCharData(Entity &e, BitStream &src, const GameDataStore &data);
void markEntityForDbStore(Entity *e, DbStoreFlags f);
void unmarkEntityForDbStore(Entity *e, DbStoreFlags f);
void resetEntityForUpdate(Entity *e);
void revivePlayer(Entity &e, ReviveLevel lvl);
