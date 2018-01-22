#pragma once

#include "Entity.h"
#include "Character.h"

/*
 * Entity Methods
 */
// Getters
uint32_t    getIdx(const Entity &e);
uint32_t    getDbId(const Entity &e);
uint32_t    getTargetIdx(const Entity &e);
uint32_t    getAssistTargetIdx(const Entity &e);

// Setters
void    setDbId(Entity &e, uint8_t val);
void    setBackupSpd(Entity &e, const float &val);
void    setJumpHeight(Entity &e, const float &val);
void    setUpdateID(Entity &e, const uint8_t &val);
void    setu1(Entity &e, const int &val);
void    setu2(Entity &e, const int &val);
void    setu3(Entity &e, const int &val);
void    setu4(Entity &e, const int &val);
void    setu5(Entity &e, const int &val);
void    setu6(Entity &e, const int &val);

// Toggles
void    toggleFly(Entity &e);
void    toggleStunned(Entity &e);
void    toggleJumppack(Entity &e);
void    toggleControlsDisabled(Entity &e);
void    toggleFullUpdate(Entity &e);
void    toggleControlId(Entity &e);

// Misc Methods
void    charUpdateDB(Entity *e);

/*
 * Character Methods
 */
// Getters
uint32_t      getLevel(const Character &c);
uint32_t      getCombatLevel(const Character &c);
uint32_t      getHP(const Character &c);
uint32_t      getEnd(const Character &c);
uint64_t      getLastCostumeId(const Character &c);
QString &     getOrigin(const Character &c);
QString &     getClass(const Character &c);
QString &     getMapName(const Character &c);
uint32_t      getXP(const Character &c);
uint32_t      getDebt(const Character &c);
uint32_t      getPatrolXP(const Character &c);
QString &     getTitles(const Character &c);
uint32_t      getInf(const Character &c);
QString &     getDescription(const Character &c);
QString &     getBattleCry(const Character &c);
QString &     getAlignment(const Character &c);


// Setters
void    setLevel(Character &c, uint32_t val);
void    setCombatLevel(Character &c, uint32_t val);
void    setHP(Character &c, uint32_t val);
void    setEnd(Character &c, uint32_t val);
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
void    toggleLFG(Character &c);
