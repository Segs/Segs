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

class GameServer  : public EventProcessor
{
        class PrivateData;
public:
                                ~GameServer();
                                GameServer(int id);
        bool                    ReadConfigAndRestart();
        bool                    Run(void);
        bool                    ShutDown(const QString &reason="No particular reason") ;
        const ACE_INET_Addr &   getAddress();

        QString                 getName(void);
        uint8_t                 getId(void);
        uint16_t                getCurrentPlayers(void);
        uint16_t                getMaxPlayers();
        CharacterDatabase *     getDb();
        int                     createLinkedAccount(uint64_t auth_account_id,const std::string &username); // Part of exposed db interface.
        EventProcessor *        event_target();
        int                     getMaxCharacterSlots() const;
        GameServerData &        runtimeData();
protected:
        void                    dispatch(SEGSEvent *ev) override;
        uint32_t                GetClientCookie(const ACE_INET_Addr &client_addr); // returns a cookie that will identify user to the gameserver
        std::unique_ptr<PrivateData> d;
};
