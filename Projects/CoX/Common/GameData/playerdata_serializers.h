/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once

#include <stdint.h>

struct PlayerData;

template<class Archive>
void serialize(Archive &archive, PlayerData &fr, uint32_t const version);


