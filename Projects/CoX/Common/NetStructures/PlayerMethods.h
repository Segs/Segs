#pragma once

#include "Entity.h"
#include "Character.h"
//#include "../Projects/CoX/Servers/MapServer/MapServerData.h"

/*
 * Entity Methods
 */
uint32_t    getIdx(Entity &e);
uint32_t    getDbId(Entity &e);
void        setDbId(Entity &e, uint8_t val);

void    toggleFly(Entity &e);
void    toggleStunned(Entity &e);
void    toggleJumppack(Entity &e);
void    setBackupSpd(Entity &e, const float &val);
void    setJumpHeight(Entity &e, const float &val);
void    toggleControlsDisabled(Entity &e);
void    setUpdateID(Entity &e, const uint8_t &val);
void    setu1(Entity &e, const int &val);
void    setu2(Entity &e, const int &val);
void    setu3(Entity &e, const int &val);
void    setu4(Entity &e, const int &val);
void    setu5(Entity &e, const int &val);
void    setu6(Entity &e, const int &val);

/*
 * Character Methods
 */
uint32_t            getLevel(Character &src);
void                setLevel(Character &src, uint32_t val);
uint32_t            getHP(Character &src);
void                setHP(Character &src, uint32_t val);
uint32_t            getEnd(Character &src);
void                setEnd(Character &src, uint32_t val);
uint64_t            getLastCostumeId(Character &src);
void                setLastCostumeId(Character &src, uint64_t val);
const QString &     getOrigin(Character &src);
const QString &     getClass(Character &src);
const QString &     getMapName(Character &src);
void                setMapName(Character &src, const QString &val);
uint32_t            getXP(Character &src);
void                setXP(Character &src, uint32_t val);
const QString &     getTitles(Character &src);
void                setTitles(Character &src, bool prefix = false, QString generic = "", QString origin = "", QString special = "");
void                toggleAFK(Character &src, const QString &msg = "");
void                toggleLFG(Character &src);
uint32_t            getInf(Character &src);
void                setInf(Character &src, uint32_t val);
