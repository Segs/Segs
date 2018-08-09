/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Entity.h"

struct PosUpdate;

struct BinTreeEntry {
    int16_t x=0,y=0,z=0;
    uint8_t m_has_height :1;
    uint8_t m_has_other  :1;
};

struct BinTreeBase {
    BinTreeEntry m_interp_pos[7];
};

extern  bool        g_interpolating;
extern  uint8_t     g_interpolation_level;
extern  uint8_t     g_interpolation_bits;

void interpolate_pos_updates(Entity *e, std::array<BinTreeEntry,7> &server_pos_update);
std::array<BinTreeEntry,7> testEncVec(std::vector<PosUpdate> vals, float min_error);
void entCalcInterp(Entity *ent, glm::mat4 *mat4, uint32_t time, glm::vec3 *next_pyr);
int storeBinTreesResult(BitStream &bs,const std::array<BinTreeEntry,7> &bintree);
int runTest(Entity &e);
