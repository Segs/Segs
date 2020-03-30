/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
class Entity;
class BitStream;
struct ClientEntityStateBelief;
void serializeto(const Entity & src, ClientEntityStateBelief &belief, BitStream &bs);
void sendBuffs(const Entity &src,BitStream &bs);
