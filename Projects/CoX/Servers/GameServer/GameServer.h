/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "GameLink.h"
#include "MapServer/EntityStorage.h"

#include <memory>

class CharacterDatabase;
class GameServerData;

class GameServer  : public EventProcessor
{
        class PrivateData;
public:
                                ~GameServer() override;
                                GameServer(int id);
        bool                    ReadConfigAndRestart();
        bool                    ShutDown() ;
        const ACE_INET_Addr &   getAddress();

        QString                 getName();
        uint8_t                 getId();
        uint16_t                getCurrentPlayers();
        uint16_t                getMaxPlayers();
        int                     getMaxCharacterSlots() const;
protected:
        int                     handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask) override;
        void                    dispatch(SEGSEvents::Event *ev) override;
        std::unique_ptr<PrivateData> d;
};
