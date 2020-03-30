/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "GameLink.h"

#include <memory>

class CharacterDatabase;
class GameServerData;

class GameServer final : public EventProcessor
{
        class PrivateData;
public:
                                IMPL_ID(GameServer)
                                ~GameServer() override;
                                GameServer(int id);
        bool                    ReadConfigAndRestart();
        const ACE_INET_Addr &   getAddress();

        uint8_t                 getId();
        uint16_t                getCurrentPlayers();
        uint16_t                getMaxPlayers();
        int                     getMaxCharacterSlots() const;
protected:
        int                     handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask) override;
        void                    dispatch(SEGSEvents::Event *ev) override;
        void                    serialize_from(std::istream &is) override;
        void                    serialize_to(std::ostream &os) override;
        std::unique_ptr<PrivateData> d;
};
