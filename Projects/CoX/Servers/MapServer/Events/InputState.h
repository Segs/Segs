/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#pragma once
// if this file is included stand-alone this will pull in common definitions
#include "MapEvents.h"
#include "CommonNetStructures.h"
#include "Entity.h"
class InputState : public MapLinkEvent
{
public:
    InputStateStorage m_data;
public:
    InputState() : MapLinkEvent(MapEventTypes::evInputState)
    {}
    void serializeto(BitStream &) const;
    void partial_2(BitStream &bs);
    void extended_input(BitStream &bs);
    void serializefrom(BitStream &bs);
    void recv_client_opts(BitStream &bs);
    osg::Vec3 &pyr() {return m_data.camera_pyr;}
};
