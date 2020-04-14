/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <stdint.h>
#include <QtCore/QString>

class Character;
struct EntityData;
struct Friend;

/*
 * Character Methods
 */
// Getters
uint32_t            getLevel(const Character &c);
uint32_t            getCombatLevel(const Character &c);
uint32_t            getSecurityThreat(const Character &c);
float               getHP(const Character &c);
float               getEnd(const Character &c);
float               getMaxHP(const Character &c);
float               getMaxEnd(const Character &c);
uint32_t            getCurrentCostumeIdx(const Character &c);
const QString &     getOrigin(const Character &c);
const QString &     getClass(const Character &c);
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
const QString &     getLastOnline(const Character &c);
//======================================================
// Accessor functions
//======================================================
void    setLevel(Character &c, uint32_t val);
void    setCombatLevel(Character &c, uint32_t val);
void    setSecurityThreat(Character &c, uint32_t val);
void    setHP(Character &c, float val);
void    setEnd(Character &c, float val);
void    setHPToMax(Character &c);
void    setEndToMax(Character &c);
void    setCurrentCostumeIdx(Character &c, uint32_t idx);
void    setXP(Character &c, uint32_t val);
void    setDebt(Character &c, uint32_t val);
void    setTitles(Character &c, bool prefix = false, QString generic = "", QString origin = "", QString special = "");
void    setInf(Character &c, uint32_t val);
void    setDescription(Character &c, QString val);
void    setBattleCry(Character &c, QString val);
void    setAFK(Character &c, const bool is_afk, QString msg = "");
bool    isAFK(Character &c);
void    updateLastOnline(Character &c);
void    initializeCharacter(Character &c);

// Toggles
void    toggleAFK(Character &c, QString msg = "");

/*
 * Titles -- TODO: get titles from texts/English/titles_def
 */
const QString &getGenericTitle(uint32_t val);
const QString &getOriginTitle(uint32_t val);
