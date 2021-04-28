/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "Messages/Map/ChatMessage.h"
#include "Messages/Map/Browser.h"
#include "Messages/Map/FloatingDamage.h"
#include "Messages/Map/FloatingInfo.h"
#include "Messages/Map/FloatingInfoStyles.h"
#include "Messages/Map/InfoMessageCmd.h"
#include "Messages/Map/StandardDialogCmd.h"
#include "Messages/Map/StoresEvents.h"
#include "DataHelpers.h"
#include "MapSceneGraph.h"
#include "MapInstance.h"
#include "MessageHelpers.h"
#include "ScriptingEngine.h"
#include "ScriptingEnginePrivate.h"

#include "Components/TimeHelpers.h"

using namespace SEGSEvents;

template<class T>
static void destruction_is_an_error(T &/*v*/)
{
    assert(false);
}

void ScriptingEngine::register_GenericTypes()
{
    m_private->m_lua.new_usertype<QString>( "QString",
        sol::constructors<QString(), QString(const char*)>()
    );

    m_private->m_lua["include_lua"] = [this](const char *path) -> bool { return m_private->performInclude(path); };

    m_private->m_lua.new_usertype<glm::vec3>( "vec3",
        sol::constructors<glm::vec3(), glm::vec3(float,float,float)>(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z
    );

    m_private->m_lua["DateTime"] = m_private->m_lua.create_table();
    m_private->m_lua["DateTime"]["SecsSince2000Epoch"] = []()
    {
        return getSecsSince2000Epoch();
    };

    m_private->m_lua.new_usertype<Entity>( "Entity",
        "new",    sol::no_constructor, // not constructible from the script side.
        sol::meta_function::garbage_collect, sol::destructor( destruction_is_an_error<Entity> ),
        "abort_logout",  abortLogout,
        "begin_logout",  &Entity::beginLogout
    );

    m_private->m_lua.new_usertype<MapSceneGraph>( "MapSceneGraph",
        "new", sol::no_constructor // The client links are not constructible from the script side.
    );

    m_private->m_lua.script("function ErrorHandler(msg) return \"Lua call error:\"..msg end");
    m_private->m_lua["printDebug"] = [](const char* msg)
    {
        qCDebug(logScripts) << msg;
    };

    m_private->m_lua["ParseContactButton"] = [this](uint32_t button_id)
      {
          QString result;
          result = contactLinkHash.key(button_id);
          if(result.isEmpty())
          {
              result = "Not found";
          }

          qCDebug(logScripts) << "ParseContactButton Result: " << result;
          return sol::make_object(m_private->m_lua, qPrintable(result));
      };


    //MapInstance
    m_private->m_lua.new_usertype<MapInstance>( "MapInstance",
        "new", sol::no_constructor // Not constructible from the script side.
         );

    m_private->m_lua["MapInstance"]["NpcMessage"] = [this](const char* channel, int entityIdx, const char* message)
    {
        QString msg = QString::fromUtf8(message);
        QString ch = QString::fromUtf8(channel);
        npcSendMessage(*mi, ch, entityIdx, msg);
    };

    m_private->m_lua["MapInstance"]["SetOnTickCallback"] = [this](uint32_t entityIdx, std::function<void(int64_t,int64_t,int64_t)> callback)
    {
        e = getEntity(mi, entityIdx);
        if(e != nullptr){
            LuaTimer timer;
            timer.m_entity_idx = entityIdx;
            timer.m_on_tick_callback = callback;
            mi->m_lua_timers.push_back(timer);
        }
    };

    m_private->m_lua["MapInstance"]["StartTimer"] = [this](uint32_t entityIdx)
    {
        e = getEntity(mi, entityIdx);
        if(e != nullptr)
           mi->startLuaTimer(entityIdx);
    };

    m_private->m_lua["MapInstance"]["StopTimer"] = [this](uint32_t entityIdx)
    {
        e = getEntity(mi, entityIdx);
        if(e != nullptr)
            mi->stopLuaTimer(entityIdx);
    };

    m_private->m_lua["MapInstance"]["ClearTimer"] = [this](uint32_t entityIdx)
    {
        e = getEntity(mi, entityIdx);
        if(e != nullptr)
            mi->clearLuaTimer(entityIdx);
    };

    //MapClientSession
    m_private->m_lua.new_usertype<MapClientSession>( "MapClientSession",
        "new", sol::no_constructor, // The client links are not constructible from the script side.
        "m_ent",  sol::readonly( &MapClientSession::m_ent ),
        "admin_chat_message", sendChatMessage);

    m_private->m_lua["MapClientSession"]["MapMenu"] = [this]()
    {
        showMapMenu(*cl);
    };

    m_private->m_lua["MapClientSession"]["Simple_dialog"] = [this](const char *dlgtext)
    {
        cl->addCommandToSendNextUpdate(std::make_unique<StandardDialogCmd>(dlgtext));
    };

    m_private->m_lua["MapClientSession"]["Browser"] = [this](const char *content)
    {
        cl->addCommand<Browser>(content);
    };

    m_private->m_lua["MapClientSession"]["Contact_dialog"] = [this](const char *message, sol::as_table_t<std::map<std::string, sol::as_table_t<std::vector<std::string>>>> buttons)
    {
        std::vector<ContactEntry> active_contacts;
        const auto& listMap = buttons.value();

        for (const auto& kvp : listMap)
        {
            const std::vector<std::string>& strings = kvp.second.value();
            int count = 0;
            ContactEntry con;
            for (const auto& s: strings)
            {
                if(count == 0)
                    con.m_response_text = QString::fromStdString(s);
                else
                    con.m_link = static_cast<uint32_t>(contactLinkHash.find(QString::fromStdString(s)).value());

                count++;
            }
            active_contacts.push_back(con);
        }
        sendContactDialog(*cl, message, active_contacts);
    };

    m_private->m_lua["MapClientSession"]["SendFloatingInfo"] = [this](int message_type)
    {
        FloatingInfoMsgKey f_info_message = static_cast<FloatingInfoMsgKey>(message_type);
        QString message = FloatingInfoMsg.find(f_info_message).value();
        cl->addCommand<FloatingInfo>(cl->m_ent->m_idx, message, FloatingInfo_Attention , 4.0);
    };

    m_private->m_lua["MapClientSession"]["ForceEntityLocation"] = [this](uint32_t entityidx, glm::vec3 loc, glm::vec3 ori)
    {
        e = getEntity(cl, entityidx);
        if(e != nullptr)
        {
            forcePosition(*e, loc);
            forceOrientation(*e, ori);
            QString msg = QString("Setting entiry %1 orientation to x: %2 y: %3 z: %4").arg(entityidx).arg(ori.x).arg(ori.y).arg(ori.z);
            qCDebug(logScripts) << msg;
        }
        else
            qCDebug(logScripts) << "Entity "<< entityidx << " not found";
    };

    m_private->m_lua["MapClientSession"]["SetNpcStore"] = [this](uint32_t entityidx, const char* store_name, int item_count)
    {
        e = getEntity(cl, entityidx);
        e->m_is_store = true;
        QString stores = store_name;

        if(stores.contains(','))
        {
            QStringList parts = stores.split(",");
            for (const QString &s: parts)
            {
                e->m_store_items.push_back(StoreItem(s.trimmed(), item_count));
            }
        }
        else
            e->m_store_items.push_back(StoreItem(store_name, item_count));
    };

    m_private->m_lua["MapClientSession"]["SendLocation"] = [this](const char* name, glm::vec3 loc){
        VisitLocation location;
        location.m_location_name = QString::fromUtf8(name);
        location.m_pos = loc;
        sendLocation(*cl, location);
    };

    m_private->m_lua["MapClientSession"]["OpenStore"] = [this](uint32_t entityidx)
    {
         e = getEntity(cl, entityidx);
         Store store;
         store.m_npc_idx = entityidx;
         store.m_store_Items = e->m_store_items;
         cl->addCommand<StoreOpen>(store);
    };

    m_private->m_lua["MapClientSession"]["SendInfoMessage"] = [this](int channel, const char* message)
    {
        sendInfoMessage(static_cast<MessageChannel>(channel), QString::fromUtf8(message), *cl);
    };

    m_private->m_lua["MapClientSession"]["DeveloperConsoleOutput"] = [this](const char* message)
    {
        QString msg = QString::fromUtf8(message);
        sendDeveloperConsoleOutput(*cl, msg);
    };

    m_private->m_lua["MapClientSession"]["ClientConsoleOutput"] = [this](const char* message)
    {
        QString msg = QString::fromUtf8(message);
        sendClientConsoleOutput(*cl, msg);
    };

    m_private->m_lua["MapClientSession"]["NpcMessage"] = [this](const char* channel, int entityIdx, const char* message)
    {
        QString msg = QString::fromUtf8(message);
        QString ch = QString::fromUtf8(channel);
        npcSendMessage(*cl, ch, entityIdx, msg);
    };

    m_private->m_lua["MapClientSession"]["SetOnTickCallback"] = [this](uint32_t entityIdx, std::function<void(int64_t,int64_t,int64_t)> callback)
    {
        e = getEntity(cl, entityIdx);
        if(e != nullptr)
        {
            LuaTimer timer;
            timer.m_entity_idx = entityIdx;
            timer.m_on_tick_callback = callback;
            mi->m_lua_timers.push_back(timer);
        }
    };

    m_private->m_lua["MapClientSession"]["StartTimer"] = [this](uint32_t entityIdx)
    {
        e = getEntity(cl, entityIdx);
        if(e != nullptr)
        {
           int count = 0;
           for(auto &t: mi->m_lua_timers)
           {
               if(t.m_entity_idx == entityIdx)
                   break;

               ++count;
           }
           mi->m_lua_timers[count].m_is_enabled = true;
        }
    };

    m_private->m_lua["MapClientSession"]["StopTimer"] = [this](uint32_t entityIdx)
    {
        e = getEntity(cl, entityIdx);
        if(e != nullptr)
        {
           int count = 0;
           for(auto &t: mi->m_lua_timers)
           {
               if(t.m_entity_idx == entityIdx)
                   break;

               ++count;
           }
           mi->m_lua_timers[count].m_is_enabled = true;
        }
    };
}
