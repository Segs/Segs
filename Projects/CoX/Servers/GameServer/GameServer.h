/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#pragma once

#include "Common/Servers/ServerEndpoint.h"
#include "GameLink.h"
#include "Common/Servers/GameServerInterface.h"
#include "Entity.h"
#include "MapServer/EntityStorage.h"

#include <string>
#include <memory>

class CharacterDatabase;
class GameServerData;

class GameLinkEndpoint : public ServerEndpoint
{
public:
    GameLinkEndpoint(const ACE_INET_Addr &local_addr) : ServerEndpoint(local_addr) {}
    ~GameLinkEndpoint()=default;
protected:
    ILink *createLink(EventProcessor *down) override
    {
        return new GameLink(down,this);
    }
};

class GameServer : public IGameServer
{
        class PrivateData;
public:
                                ~GameServer();
                                GameServer(int id);
        bool                    ReadConfig() override;
        bool                    Run(void) override;
        bool                    ShutDown(const QString &reason="No particular reason") override;
        void                    Online(bool s );
        bool                    Online(void) override;
        const ACE_INET_Addr &   getAddress() override;

        QString                 getName(void) override;
        uint8_t                 getId(void) override;
        uint16_t                getCurrentPlayers(void) override;
        uint16_t                getMaxPlayers() override;
        CharacterDatabase *     getDb();
        int                     createLinkedAccount(uint64_t auth_account_id,const std::string &username); // Part of exposed db interface.
        EventProcessor *        event_target() override;
        int                     getMaxCharacterSlots() const override;
        GameServerData &        runtimeData();
protected:
        uint32_t                GetClientCookie(const ACE_INET_Addr &client_addr); // returns a cookie that will identify user to the gameserver
        std::unique_ptr<PrivateData> d;
};
extern GameServer *g_GlobalGameServer;
