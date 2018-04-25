/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include <stdint.h>

class QString;
struct Friend;
struct FriendsList;
struct Sidekick;
struct CharacterData;

template<class Archive>
void serialize(Archive &archive, Friend &fr, uint32_t const version);
template<class Archive>
void serialize(Archive &archive, FriendsList &fl, uint32_t const version);
template<class Archive>
void serialize(Archive &archive, Sidekick &sk, uint32_t const version);
template<class Archive>
void serialize(Archive &archive, CharacterData &cd, uint32_t const version);

void saveTo(const CharacterData &target,const QString &baseName,bool text_format=false);
