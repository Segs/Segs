/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */
#include "ScriptingEngine.h"

#include "DataHelpers.h"
#include "MessageHelpers.h"
#include "MapInstance.h"
#include "MapSceneGraph.h"
#include "GameData/playerdata_definitions.h"
#include "Messages/Map/Browser.h"
#include "Messages/Map/ChatMessage.h"
#include "Messages/Map/FloatingDamage.h"
#include "Messages/Map/StandardDialogCmd.h"
#include "Messages/Map/StoresEvents.h"
#include "Messages/Map/InfoMessageCmd.h"
#include "Messages/Map/SendVisitLocation.h"
#include "Common/GameData/Character.h"
#include "Common/GameData/CharacterHelpers.h"
#include "Common/GameData/Contact.h"
#include "Common/GameData/Entity.h"
#include "Common/GameData/Contact.h"
#include "Common/GameData/Task.h"
#include "TimeHelpers.h"
#define SOL_CHECK_ARGUMENTS 1
#include <lua/lua.hpp>
#include <sol2/sol.hpp>

#include <QtCore/QFileInfo> // for include support
#include <QtCore/QDir>
#include <QtCore/QDebug>

using namespace SEGSEvents;
static constexpr const int MAX_INCLUDED_FILE_SIZE=1024*1024; // 1MB of lua code should be enough for anyone :P
int luaopen_package(lua_State *)
{
    assert(false && "functionality removed");
    return 0;
}

int luaopen_io(lua_State *)
{
    assert(false && "functionality removed");
    return 0;
}

int luaopen_os(lua_State *)
{
    assert(false && "functionality removed");
    return 0;
}

struct ScriptingEngine::ScriptingEnginePrivate
{
    ScriptingEnginePrivate()
    {
        m_lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::table, sol::lib::string, sol::lib::math,
                             sol::lib::utf8, sol::lib::debug);
    }

    bool performInclude(const char *path)
    {
        if(m_restricted_include_dir.isEmpty())
            return false;
        QString full_path = QDir(m_restricted_include_dir).filePath(QDir::cleanPath(path));
        if(m_alread_included_and_ran.contains(full_path))
            return true;
        QFileInfo include_info(full_path);
        if(!include_info.exists() || !include_info.isReadable() || !include_info.isFile())
            return false;
        QFile content_file(full_path);
        if(!content_file.open(QFile::ReadOnly))
            return false;
        QByteArray script_contents = content_file.read(MAX_INCLUDED_FILE_SIZE);
        if(script_contents.size()==MAX_INCLUDED_FILE_SIZE)
            return false;

        sol::load_result load_res = m_lua.load(script_contents.toStdString(),qPrintable(include_info.filePath()));
        if(!load_res.valid())
        {
            sol::error err = load_res;
            qWarning() << err.what();
            return false;
        }
        // Run the included code
        sol::protected_function_result script_result = load_res();
        if(!script_result.valid())
        {
            sol::error err = script_result;
            qWarning() << err.what();
            return false;
        }
        m_alread_included_and_ran.insert(full_path);
        return true;
    }

    sol::state m_lua;
    QString m_restricted_include_dir;
    QSet<QString> m_alread_included_and_ran;
};

ScriptingEngine::ScriptingEngine() : m_private(new ScriptingEnginePrivate)
{
    m_private->m_lua["include_lua"] = [this](const char *path) -> bool { return m_private->performInclude(path); };
}

ScriptingEngine::~ScriptingEngine() = default;


template<class T>
static void destruction_is_an_error(T &/*v*/)
{
    assert(false);
}


void ScriptingEngine::registerTypes()
{

    m_private->m_lua.new_usertype<Clue>("Clue",
        sol::constructors<Clue()>(),
        "name", sol::property(&Clue::getName, &Clue::setName),
        "displayName", sol::property(&Clue::getDisplayName, &Clue::setDisplayName),
        "detail", sol::property(&Clue::getDetailText, &Clue::setDetailText),
        "iconFile", sol::property(&Clue::getIconFile, &Clue::setIconFile)
    );

    m_private->m_lua.new_usertype<Contact>( "Contact",
        // 3 constructors
        sol::constructors<Contact()>(),
        // gets or set the value using member variable syntax
        "name", sol::property(&Contact::getName, &Contact::setName),
        "locationDescription", sol::property(&Contact::getLocationDescription, &Contact::setLocationDescription),
        "npcId", &Contact::m_npc_id,
        "currentStanding", &Contact::m_current_standing,
        "contactIdx", &Contact::m_contact_idx,
        "notifyPlayer", &Contact::m_notify_player,
        "taskIndex", &Contact::m_task_index,
        "hasLocation", &Contact::m_has_location,
        "location", &Contact::m_location,
        "confidantThreshold", &Contact::m_confidant_threshold,
        "friendThreshold", &Contact::m_friend_threshold,
        "completeThreshold", &Contact::m_complete_threshold,
        "canUseCell", &Contact::m_can_use_cell,
        "contactId", &Contact::m_contact_idx,
        "dialogScreenIdx", &Contact::m_dlg_screen,
        "settingTitle", &Contact::m_setting_title
    );

    m_private->m_lua.new_usertype<Destination>("Destination",
        sol::constructors<Destination()>(),
        "pointIdx", &Destination::point_idx,
        "location", &Destination::location,
        "name", sol::property(&Destination::getLocationName, &Destination::setLocationName),
        "mapName", sol::property(&Destination::getLocationMapName, &Destination::setLocationMapName)
    );

    m_private->m_lua.new_usertype<HideAndSeek>( "HideAndSeek",
        sol::constructors<HideAndSeek()>(),
        "foundCount", &HideAndSeek::m_found_count
    );

    m_private->m_lua.new_usertype<Hunt>( "Hunt",
        sol::constructors<Hunt()>(),
        "type", sol::property(&Hunt::getTypeString, &Hunt::setTypeString),
        "count", &Hunt::m_count
    );

    m_private->m_lua.new_usertype<QString>( "QString",
        sol::constructors<QString(), QString(const char*)>()
    );

    m_private->m_lua.new_usertype<RelayRaceResult>( "RelayRaceResult",
        sol::constructors<RelayRaceResult()>(),
        "segment", &RelayRaceResult::m_segment,
        "lastTime", &RelayRaceResult::m_last_time,
        "bestTime", &RelayRaceResult::m_best_time
    );

    m_private->m_lua.new_usertype<Souvenir>("Souvenir",
        sol::constructors<Souvenir()>(),
        "name", sol::property(&Souvenir::getName, &Souvenir::setName),
        "description", sol::property(&Souvenir::getDescription, &Souvenir::setDescription),
        "icon", sol::property(&Souvenir::getIcon, &Souvenir::setIcon)
        //Server will set the index for this
        //"souvenirIdx", &Souvenir::m_idx
    );

    m_private->m_lua.new_usertype<TaskEntry>("TaskEntry",
          sol::constructors<TaskEntry()>(),
          "dbId", &TaskEntry::m_db_id,
          "taskList", &TaskEntry::m_task_list,
          "resetSelectedTask", &TaskEntry::m_task_list
          );

      m_private->m_lua.new_usertype<Task>("Task",
          sol::constructors<Task()>(),
          "dbId", &Task::m_db_id,
          "description", sol::property(&Task::getDescription, &Task::setDescription),
          "owner", sol::property(&Task::getOwner, &Task::setOwner),
          "detail", sol::property(&Task::getDetail, &Task::setDetail),
          "state", sol::property(&Task::getState, &Task::setState),
          "isComplete", &Task::m_is_complete,
          "inProgressMaybe", &Task::m_in_progress_maybe,
          "hasLocation", &Task::m_has_location,
          "isDetailInvalid", &Task::m_detail_invalid,
          "location", &Task::m_location,
          "finishTime", &Task::m_finish_time,
          "taskIdx", &Task::m_task_idx,
          "isAbandoned", &Task::m_is_abandoned,
          "unknownInt1", &Task::m_unknown_1,
          "unknownInt2", &Task::m_unknown_2,
          "boardTrain", &Task::m_board_train
      );

      m_private->m_lua.new_usertype<glm::vec3>( "vec3",
          sol::constructors<glm::vec3(), glm::vec3(float,float,float)>(),
          "x", &glm::vec3::x,
          "y", &glm::vec3::y,
          "z", &glm::vec3::z
      );


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

      m_private->m_lua["DateTime"] = m_private->m_lua.create_table();
      m_private->m_lua["DateTime"]["SecsSince2000Epoch"] = [this]()
      {
          return getSecsSince2000Epoch();
      };

    //MapInstance
    m_private->m_lua.new_usertype<MapInstance>( "MapInstance",
        "new", sol::no_constructor // Not constructible from the script side.
         );

    m_private->m_lua["MapInstance"]["AddNpc"] = [this](const char* npc_def, glm::vec3 &loc, glm::vec3 &ori, int variation, const char* npc_name)
    {
        MapInstance *mi = m_private->m_lua["map"];
        QString npc_def_name = QString::fromUtf8(npc_def);
        QString name = QString::fromUtf8(npc_name);
        addNpcWithOrientation(*mi, npc_def_name, loc, variation, ori, name);
    };

    m_private->m_lua["MapInstance"]["AddEnemy"] = [this](const char* npc_def, glm::vec3 &loc, glm::vec3 &ori, int variation, const char* npc_name, int level, const char* faction_name, int rank)
    {
        MapInstance *mi = m_private->m_lua["map"];
        QString npc_def_name = QString::fromUtf8(npc_def);
        QString name = QString::fromUtf8(npc_name);
        QString faction = QString::fromUtf8(faction_name);
        addEnemy(*mi, npc_def_name, loc, variation, ori, name, level, faction, rank);
    };


    m_private->m_lua["MapInstance"]["RemoveNpc"] = [this](int entityIdx)
    {
        MapInstance *mi = m_private->m_lua["map"];
        Entity *e = getEntity(mi, entityIdx);
        if(e != nullptr)
        {
            mi->m_entities.removeEntityFromActiveList(e);
        }

    };

    m_private->m_lua["MapInstance"]["NpcMessage"] = [this](const char* channel, int entityIdx, const char* message)
    {
        MapInstance *mi = m_private->m_lua["map"];
        QString msg = QString::fromUtf8(message);
        QString ch = QString::fromUtf8(channel);
        npcSendMessage(*mi, ch, entityIdx, msg);
    };

    m_private->m_lua["MapInstance"]["SetOnTickCallback"] = [this](int entityIdx, std::function<void(int64_t,int64_t,int64_t)> callback)
    {
        MapInstance *mi = m_private->m_lua["map"];
        Entity* e = getEntity(mi, entityIdx);
        if(e != nullptr){
            LuaTimer timer;
            timer.m_entity_idx = entityIdx;
            timer.m_on_tick_callback = callback;
            mi->m_lua_timers.push_back(timer);
        }
    };

    m_private->m_lua["MapInstance"]["StartTimer"] = [this](int entityIdx)
    {
        MapInstance *mi = m_private->m_lua["map"];
        Entity* e = getEntity(mi, entityIdx);
        if(e != nullptr)
        {
           mi->startTimer(entityIdx);
        }

    };

    m_private->m_lua["MapInstance"]["StopTimer"] = [this](int entityIdx)
    {
        MapInstance *mi = m_private->m_lua["map"];
        Entity* e = getEntity(mi, entityIdx);
        if(e != nullptr)
        {
            mi->stopTimer(entityIdx);
        }
    };

    m_private->m_lua["MapInstance"]["ClearTimer"] = [this](int entityIdx)
    {
        MapInstance *mi = m_private->m_lua["map"];
        Entity* e = getEntity(mi, entityIdx);
        if(e != nullptr)
        {
            mi->clearTimer(entityIdx);
        }
    };

    //MapClientSession
    m_private->m_lua.new_usertype<MapClientSession>( "MapClientSession",

        "new", sol::no_constructor, // The client links are not constructible from the script side.
        "m_ent",  sol::readonly( &MapClientSession::m_ent ),
        "admin_chat_message", sendChatMessage

        );

    m_private->m_lua["MapClientSession"]["MapMenu"] = [this]()
    {
        MapClientSession *cl = m_private->m_lua["client"];
        showMapMenu(*cl);
    };
    m_private->m_lua["MapClientSession"]["Simple_dialog"] = [this](const char *dlgtext)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        cl->addCommandToSendNextUpdate(std::make_unique<StandardDialogCmd>(dlgtext));
    };
    m_private->m_lua["MapClientSession"]["Browser"] = [this](const char *content)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        cl->addCommand<Browser>(content);
    };
    m_private->m_lua["MapClientSession"]["Contact_dialog"] = [this](const char *message, sol::as_table_t<std::map<std::string, sol::as_table_t<std::vector<std::string>>>> buttons)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        std::vector<ContactEntry> active_contacts;
        const auto& listMap = buttons.source;

        for (const auto& kvp : listMap)
        {
            const std::vector<std::string>& strings = kvp.second.source;
            int count = 0;
            ContactEntry con;
            for (const auto& s: strings)
            {
                if(count == 0)
                    con.m_response_text = QString::fromStdString(s);
                else
                    con.m_link = contactLinkHash.find(QString::fromStdString(s)).value();

                count++;
            }
            active_contacts.push_back(con);
        }
        sendContactDialog(*cl, message, active_contacts);
    };
    m_private->m_lua["MapClientSession"]["SendFloatingInfo"] = [this](int message_type)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        FloatingInfoMsgKey f_info_message = static_cast<FloatingInfoMsgKey>(message_type);
        QString message = FloatingInfoMsg.find(f_info_message).value();
        cl->addCommand<FloatingInfo>(cl->m_ent->m_idx, message, FloatingInfo_Attention , 4.0);
    };
    m_private->m_lua["MapClientSession"]["ForceEntityLocation"] = [this](int entity_idx, glm::vec3 loc, glm::vec3 ori)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        Entity *e = getEntity(cl, entity_idx);
        if(e != nullptr)
        {
            forcePosition(*e, loc);
            forceOrientation(*e, ori);
            QString msg = QString("Setting entiry %1 orientation to x: %2 y: %3 z: %4").arg(entity_idx).arg(ori.x).arg(ori.y).arg(ori.z);
            qCDebug(logScripts) << msg;
        }
        else
        {
             qCDebug(logScripts) << "Entity "<< entity_idx << " not found";
        }
    };

    m_private->m_lua["MapClientSession"]["SetNpcStore"] = [this](int entity_idx, const char* store_name, int item_count)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        Entity *e = getEntity(cl, entity_idx);
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
        {
            e->m_store_items.push_back(StoreItem(store_name, item_count));
        }
    };

    m_private->m_lua["MapClientSession"]["SendLocation"] = [this](const char* name, glm::vec3 loc){
        MapClientSession *cl = m_private->m_lua["client"];
        VisitLocation location;
        location.m_location_name = QString::fromUtf8(name);
        location.m_pos = loc;
        sendLocation(*cl, location);

    };

    m_private->m_lua["MapClientSession"]["OpenStore"] = [this](int entity_idx)
    {
         MapClientSession *cl = m_private->m_lua["client"];
         Entity *e = getEntity(cl, entity_idx);
         Store store;
         store.m_npc_idx = entity_idx;
         store.m_store_Items = e->m_store_items;
         cl->addCommand<StoreOpen>(store);

    };

    m_private->m_lua["MapClientSession"]["SendInfoMessage"] = [this](int channel, const char* message)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        sendInfoMessage(static_cast<MessageChannel>(channel), QString::fromUtf8(message), *cl);
    };

    m_private->m_lua["MapClientSession"]["DeveloperConsoleOutput"] = [this](const char* message)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        QString msg = QString::fromUtf8(message);
        sendDeveloperConsoleOutput(*cl, msg);
    };

    m_private->m_lua["MapClientSession"]["ClientConsoleOutput"] = [this](const char* message)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        QString msg = QString::fromUtf8(message);
        sendClientConsoleOutput(*cl, msg);
    };

    m_private->m_lua["MapClientSession"]["NpcMessage"] = [this](const char* channel, int entityIdx, const char* message)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        QString msg = QString::fromUtf8(message);
        QString ch = QString::fromUtf8(channel);
        npcSendMessage(*cl, ch, entityIdx, msg);
    };

    m_private->m_lua["MapClientSession"]["SetOnTickCallback"] = [this](int entityIdx, std::function<void(int64_t,int64_t,int64_t)> callback)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        MapInstance *mi = cl->m_current_map;
        Entity* e = getEntity(cl, entityIdx);
        if(e != nullptr)
        {
            LuaTimer timer;
            timer.m_entity_idx = entityIdx;
            timer.m_on_tick_callback = callback;
            mi->m_lua_timers.push_back(timer);
        }
    };

    m_private->m_lua["MapClientSession"]["StartTimer"] = [this](int entityIdx)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        MapInstance *mi = cl->m_current_map;
        Entity* e = getEntity(cl, entityIdx);
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

    m_private->m_lua["MapClientSession"]["StopTimer"] = [this](int entityIdx)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        MapInstance *mi = cl->m_current_map;
        Entity* e = getEntity(cl, entityIdx);
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
    //End MapClientSession

    // Player Object
    m_private->m_lua["Player"] = m_private->m_lua.create_table(); // Empty Table aka Object
    m_private->m_lua["Player"]["ClearTarget"] = [this]()
    {
        MapClientSession *cl = m_private->m_lua["client"];
        setTarget(*cl->m_ent, cl->m_ent->m_idx);
    };
    m_private->m_lua["Player"]["CloseContactDialog"] = [this]()
    {
        MapClientSession *cl = m_private->m_lua["client"];
        sendContactDialogClose(*cl);
    };
    m_private->m_lua["Player"]["SetDebt"] = [this](const int debt)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        setDebt(*cl->m_ent->m_char, debt);
    };
    m_private->m_lua["Player"]["GiveDebt"] = [this](const int debt)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        giveDebt(*cl, debt);
    };
    m_private->m_lua["Player"]["GiveEnhancement"] = [this](const char* name, int level)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        QString e_name = QString::fromUtf8(name);
        giveEnhancement(*cl, e_name, level);
    };
    m_private->m_lua["Player"]["GiveEnd"] = [this](const float end)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        giveEnd(*cl, end);
    };
    m_private->m_lua["Player"]["SetEnd"] = [this](const float end)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        setEnd(*cl->m_ent->m_char, end);
    };
    m_private->m_lua["Player"]["SetHp"] = [this](const float hp)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        setHP(*cl->m_ent->m_char, hp);
    };
    m_private->m_lua["Player"]["GiveHp"] = [this](const float hp)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        giveHp(*cl, hp);
    };
    m_private->m_lua["Player"]["SetInf"] = [this](const int inf)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        setInf(*cl->m_ent->m_char, inf);
    };
    m_private->m_lua["Player"]["GiveInf"] = [this](const int inf)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        modifyInf(*cl, inf);
    };
    m_private->m_lua["Player"]["GiveInsp"] = [this](const char* name)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        QString e_name = QString::fromUtf8(name);
        giveInsp(*cl, e_name);
    };
    m_private->m_lua["Player"]["SetXp"] = [this](const int xp)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        setXP(*cl->m_ent->m_char, xp);
    };
    m_private->m_lua["Player"]["GiveXp"] = [this](const int xp)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        giveXp(*cl, xp);
    };
    m_private->m_lua["Player"]["SendFloatingDamage"] = [this](const int tgt_idx, const int amount)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        sendFloatingNumbers(*cl, tgt_idx, amount);
    };
    m_private->m_lua["Player"]["FaceEntity"] = [this](const int tgt_idx)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        sendFaceEntity(*cl, tgt_idx);
    };
    m_private->m_lua["Player"]["FaceLocation"] = [this](glm::vec3 &loc)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        sendFaceLocation(*cl, loc);
    };
    m_private->m_lua["Player"]["AddUpdateContact"] = [this](const Contact &contact)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        updateContactStatusList(*cl, contact);
    };
    m_private->m_lua["Player"]["SetActiveDialogCallback"] = [this](std::function<void(int)> callback)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        cl->m_ent->setActiveDialogCallback(callback);
    };
    m_private->m_lua["Player"]["RemoveContact"] = [this](const Contact &contact)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        removeContact(*cl, contact);
    };
    m_private->m_lua["Player"]["UpdateTaskDetail"] = [this](const Task &task)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        updateTaskDetail(*cl, task);
    };
    m_private->m_lua["Player"]["AddListOfTasks"] = [this](const sol::as_table_t<std::vector<Task>> task_list)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        const auto& listMap = task_list.source;
        vTaskList listToSend;
        for (const auto& kvp : listMap)
        {
            listToSend.push_back(kvp);
        }
        addListOfTasks(cl, listToSend);
    };
    m_private->m_lua["Player"]["AddUpdateTask"] = [this](const Task &task)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        sendUpdateTaskStatusList(*cl, task);
    };
    m_private->m_lua["Player"]["RemoveTask"] = [this](const Task &task)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        removeTask(*cl, task);
    };
    m_private->m_lua["Player"]["SelectTask"] = [this](const Task &task)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        selectTask(*cl, task);
    };

    m_private->m_lua["Player"]["StartMissionTimer"] = [this](const char* message, float timer)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        QString mess = QString::fromUtf8(message);
        sendMissionObjectiveTimer(*cl, mess, timer);
    };

    m_private->m_lua["Player"]["SetWaypoint"] = [this](const int point_idx, glm::vec3 loc)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        sendWaypoint(*cl, point_idx, loc);
    };
    m_private->m_lua["Player"]["LevelUp"] = [this]()
    {
        MapClientSession *cl = m_private->m_lua["client"];
        playerTrain(*cl);
    };
    m_private->m_lua["Player"]["OpenTitleMenu"] = [this]()
    {
        MapClientSession *cl = m_private->m_lua["client"];
        QString origin = getOriginTitle(*cl->m_ent->m_char);
        setTitle(*cl, origin);
    };
    m_private->m_lua["Player"]["GiveTempPower"] = [this](const char* power)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        giveTempPower(cl, power);
    };
    m_private->m_lua["Player"]["AddClue"] = [this](const Clue clue)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        addClue(*cl, clue);
    };
    m_private->m_lua["Player"]["RemoveClue"] = [this](const Clue clue)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        removeClue(*cl, clue);
    };
    m_private->m_lua["Player"]["AddSouvenir"] = [this](const Souvenir souvenir)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        addSouvenir(*cl, souvenir);
    };
    m_private->m_lua["Player"]["RemoveSouvenir"] = [this](const Souvenir souvenir)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        removeSouvenir(*cl, souvenir);
    };
    m_private->m_lua["Player"]["Revive"] = [this](const int revive_lvl)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        revive(cl, revive_lvl);
    };
    m_private->m_lua["Player"]["Respawn"] = [this](const char* loc_name)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        respawn(*cl, loc_name);
    };
    m_private->m_lua["Player"]["AddHideAndSeekPoint"] = [this](int points)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        addHideAndSeekResult(*cl, points);
    };
    m_private->m_lua["Player"]["AddRelayRaceResult"] = [this](RelayRaceResult *raceResult)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        addRelayRaceResult(*cl, *raceResult);
    };
    m_private->m_lua["Player"]["GetRelayRaceResult"] = [this](int segment)
    {
        MapClientSession *cl = m_private->m_lua["client"];
        return getRelayRaceResult(*cl, segment);
    };

}

int ScriptingEngine::loadAndRunFile(const QString &filename)
{
    sol::load_result load_res = m_private->m_lua.load_file(filename.toStdString());
    if(!load_res.valid())
    {
        sol::error err = load_res;
        qCritical() << err.what();
        return -1;
    }

    sol::protected_function_result script_result = load_res();
    if(!script_result.valid())
    {
        sol::error err = script_result;
        qCritical() << err.what();
        return -1;
    }

    return 0;
}

void ScriptingEngine::callFuncWithMapInstance(MapInstance *mi, const char *name, int arg1)
{
    m_private->m_lua["map"] = mi;
    m_private->m_lua["contactDialogButtons"] = contactLinkHash;

    callFunc(name,arg1);
}

std::string ScriptingEngine::callFuncWithClientContext(MapClientSession *client, const char *name, int arg1)
{
    m_private->m_lua["client"] = client;
    m_private->m_lua["map"] = client->m_current_map;
    m_private->m_lua["contactDialogButtons"] = contactLinkHash;

    if (client->m_ent->m_type == EntType::PLAYER)
    {
        m_private->m_lua["Player"]["entityId"] = client->m_ent->m_db_id;
        m_private->m_lua["heroName"] = qPrintable(client->m_name);
        m_private->m_lua["vContacts"] = client->m_ent->m_player->m_contacts;


        if(client->m_ent->m_player->m_tasks_entry_list.size() > 0)
        {
            m_private->m_lua["vTaskList"] = client->m_ent->m_player->m_tasks_entry_list[0].m_task_list;
        }
    }

    return callFunc(name,arg1);
}

std::string ScriptingEngine::callFuncWithClientContext(MapClientSession *client, const char *name, int arg1, glm::vec3 loc)
{
    m_private->m_lua["client"] = client;
    m_private->m_lua["map"] = client->m_current_map;
    m_private->m_lua["vContacts"] = client->m_ent->m_player->m_contacts;
    m_private->m_lua["heroName"] = qPrintable(client->m_name);
    m_private->m_lua["Player"]["entityId"] = client->m_ent->m_db_id;
    if(client->m_ent->m_player->m_tasks_entry_list.size() > 0)
    {
        m_private->m_lua["vTaskList"] = client->m_ent->m_player->m_tasks_entry_list[0].m_task_list;
    }
    return callFunc(name,arg1,loc);
}

std::string ScriptingEngine::callFuncWithClientContext(MapClientSession *client, const char *name, const char *arg1, glm::vec3 loc)
{
    m_private->m_lua["client"] = client;
    m_private->m_lua["map"] = client->m_current_map;
    m_private->m_lua["vContacts"] = client->m_ent->m_player->m_contacts;
    m_private->m_lua["heroName"] = qPrintable(client->m_name);
    m_private->m_lua["Player"]["entityId"] = client->m_ent->m_db_id;
    if(client->m_ent->m_player->m_tasks_entry_list.size() > 0)
    {
        m_private->m_lua["vTaskList"] = client->m_ent->m_player->m_tasks_entry_list[0].m_task_list;
    }
    return callFunc(name,arg1,loc);
}

std::string ScriptingEngine::callFunc(const char *name, int arg1)
{
    sol::protected_function funcwrap = m_private->m_lua[name];
    funcwrap.error_handler = m_private->m_lua["ErrorHandler"];
    if(!funcwrap.valid())
    {
        qCritical() << "Failed to retrieve script func:" << name;
        return "";
    }
    auto result = funcwrap(arg1);
    if(!result.valid())
    {
        sol::error err = result;
        qCritical() << "Failed to run script func:" << name << err.what();
        return "";
    }
    return result.get<std::string>();
}

std::string ScriptingEngine::callFunc(const char *name, int arg1, glm::vec3 loc)
{
    sol::protected_function funcwrap = m_private->m_lua[name];
    funcwrap.error_handler = m_private->m_lua["ErrorHandler"];

    if(!funcwrap.valid())
    {
        qCritical() << "Failed to retrieve script func:" << name;
        return "";
    }
    auto result = funcwrap(arg1, loc);
    if(!result.valid())
    {
        sol::error err = result;
        qCritical() << "Failed to run script func:"<<name<<err.what();
        return "";
    }
    return result.get<std::string>();
}

std::string ScriptingEngine::callFunc(const char *name, const char *arg1, glm::vec3 loc)
{
    sol::protected_function funcwrap = m_private->m_lua[name];
    funcwrap.error_handler = m_private->m_lua["ErrorHandler"];

    if(!funcwrap.valid())
    {
        qCritical() << "Failed to retrieve script func:"<<name;
        return "";
    }
    auto result = funcwrap(arg1, loc);
    if(!result.valid())
    {
        sol::error err = result;
        qCritical() << "Failed to run script func:"<<name<<err.what();
        return "";
    }
    return result.get<std::string>();
}

std::string ScriptingEngine::callFunc(const char *name, std::vector<Contact> contact_list)
{
    sol::protected_function funcwrap = m_private->m_lua[name];
    funcwrap.error_handler = m_private->m_lua["ErrorHandler"];

    if(!funcwrap.valid())
    {
        qCritical() << "Failed to retrieve script func:"<<name;
        return "";
    }
    auto result = funcwrap(contact_list);
    if(!result.valid())
    {
        sol::error err = result;
        qCritical() << "Failed to run script func:"<<name<<err.what();
        return "";
    }
     return result.get<std::string>();
}

int ScriptingEngine::runScript(MapClientSession * client, const QString &script_contents, const char *script_name)
{
    m_private->m_lua["client"] = client;
    m_private->m_lua["heroName"] = qPrintable(client->m_name);
    sol::load_result load_res=m_private->m_lua.load(script_contents.toStdString(),script_name);
    if(!load_res.valid())
    {
        sol::error err = load_res;
        sendInfoMessage(MessageChannel::ADMIN,err.what(),*client);
        return -1;
    }
    sol::protected_function_result script_result = load_res();
    if(!script_result.valid())
    {
        sol::error err = script_result;
        sendInfoMessage(MessageChannel::ADMIN,err.what(),*client);
        return -1;
    }
    return 0;
}

int ScriptingEngine::runScript(const QString &script_contents, const char *script_name)
{
    sol::load_result load_res = m_private->m_lua.load(script_contents.toStdString(), script_name);
    if(!load_res.valid())
    {
        sol::error err = load_res;
        // TODO: report error here.
        return -1;
    }
    sol::protected_function_result script_result = load_res();
    if(!script_result.valid())
    {
        sol::error err = script_result;
        // TODO: report error here.
        return -1;
    }
    return 0;
}
bool ScriptingEngine::setIncludeDir(const QString &path)
{
    QFileInfo fi(path);
    if(!fi.absoluteFilePath().startsWith(QDir::currentPath()))
    {
        qWarning() << "Includes directory must be a subdirectory of the runtime" << QDir::currentPath();
        return false;
    }
    m_private->m_restricted_include_dir = fi.absoluteFilePath();
    return true;
}

void ScriptingEngine::updateMapInstance(MapInstance * instance)
{
    m_private->m_lua["map"] = instance;
}

void ScriptingEngine::updateClientContext(MapClientSession * client)
{
    m_private->m_lua["client"] = client;
    m_private->m_lua["vContacts"] = client->m_ent->m_player->m_contacts;
    m_private->m_lua["heroName"] = qPrintable(client->m_name);
    m_private->m_lua["m_db_id"] = client->m_ent->m_db_id;
    if(client->m_ent->m_player->m_tasks_entry_list.size() > 0)
    {
        m_private->m_lua["vTaskList"] = client->m_ent->m_player->m_tasks_entry_list[0].m_task_list;
    }
}

//! @}
