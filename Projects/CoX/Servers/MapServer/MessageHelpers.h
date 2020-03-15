/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Messages/Map/MessageChannels.h"

/**
  This file is meant to provide message sending helpers
*/
class QString;
struct MapClientSession;
class Entity;

namespace SEGSEvents
{
class EntitiesResponse;
}
void sendChatMessage(MessageChannel t, const QString &msg, Entity *e, MapClientSession &tgt);
void sendInfoMessage(MessageChannel t, const QString &msg, MapClientSession &tgt);

class BitStream;
class EntityManager;
void storeEntityResponseCommands(BitStream &tgt,float time_of_day);
void storeEntityResponseOtherEntities(BitStream &tgt, EntityManager &manager, MapClientSession *self, bool incremental,
                                      bool debug);
void storeClientData(BitStream &bs,Entity *ent,bool incremental);
void storeControlState(BitStream &bs,Entity *self);
void storeEntityRemovals(BitStream &bs, EntityManager &manager, MapClientSession *self);
void storeFollowupCommands(BitStream &bs,MapClientSession *m_client);
enum class EntityUpdateMode
{
    FULL,
    INCREMENTAL,
};
void buildEntityResponse(SEGSEvents::EntitiesResponse *res, MapClientSession &to_client, EntityUpdateMode mode,
                         bool use_debug);
