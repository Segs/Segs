/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "GameCommand.h"

using namespace SEGSEvents;

//void PreUpdateCommand::serializeto(BitStream &bs) const
//{
//    bs.StorePackedBits(1, 13);
//    for(const auto &command : m_contents)
//        command->serializeto(bs);
//    bs.StorePackedBits(1,0); // finalize the command list
//}

//void PreUpdateCommand::serializefrom(BitStream &)
//{
//    assert(false);
//    // TODO: trouble, we need a second GameCommand Factory at this point !
//    //uint32_t game_command = src.GetPackedBits(1);
//}

//! @}
