/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

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
        void                    dispatch(SEGSEvent *ev) override;
        std::unique_ptr<PrivateData> d;
};
