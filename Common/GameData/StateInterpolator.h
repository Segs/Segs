/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "CommonNetStructures.h"
#include <array>

class Entity;
struct PosUpdate;

struct BinTreeEntry
{
    int16_t x=0,y=0,z=0;
    bool m_has_height = false;
    bool m_has_other  = false;
};

extern  bool        g_interpolating;
extern  uint8_t     g_interpolation_level;
extern  uint8_t     g_interpolation_bits;

void interpolatePosUpdates(Entity *e, std::array<BinTreeEntry,7> &server_pos_update);
std::array<BinTreeEntry,7> interpolateBinTree(std::array<PosUpdate, 64> vals, float min_error);
void entCalcInterp(Entity *ent, glm::mat4 *mat4, uint32_t time, glm::vec3 *next_pyr);
int storeBinTreesResult(BitStream &bs, const std::array<BinTreeEntry, 7> &bintree);
int runTest(Entity &e);
