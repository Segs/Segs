/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "MapEvents.h"
#include "Common/GameData/clientoptions_definitions.h"

class SaveClientOptions final : public MapLinkEvent
{
public:
    ClientOptions data;
    SaveClientOptions() : MapLinkEvent(MapEventTypes::evSaveClientOptions) {}
    void serializeto(BitStream &bs) const override;
    void serializefrom(BitStream &bs) override;
};
