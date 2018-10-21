/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "Character.h"
#include "CharacterHelpers.h"
#include "Contact.h"
#include "DataHelpers.h"
#include "ScriptingEngine.h"
#include "MapClientSession.h"
#include "MapInstance.h"
#include "MapSceneGraph.h"
#include "Entity.h"

#include "Events/Browser.h"
#include "Events/ChatMessage.h"
#include "Events/FloatingDamage.h"
#include "Events/StandardDialogCmd.h"
#include "Events/InfoMessageCmd.h"
#include "Common/NetStructures/Entity.h"
#include "Common/NetStructures/Contact.h"
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
        QString full_path=QDir(m_restricted_include_dir).filePath(QDir::cleanPath(path));
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
        {
            return false;
        }
        sol::load_result load_res=m_lua.load(script_contents.toStdString(),qPrintable(include_info.filePath()));
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
    m_private->m_lua.new_usertype<glm::vec3>( "vec3",
        sol::constructors<glm::vec3(), glm::vec3(float,float,float)>(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z
    );
    m_private->m_lua.new_usertype<Contact>( "Contact",
        // 3 constructors
        sol::constructors<Contact()>(),
        // gets or set the value using member variable syntax
        "name", sol::property(&Contact::getName, &Contact::setName),
        "locationDescription", sol::property(&Contact::getLocationDescription, &Contact::setLocationDescription),
        "npcId", &Contact::m_npc_id,
        "currentStanding", &Contact::m_current_standing,
        "notifyPlayer", &Contact::m_notify_player,
        "taskIndex", &Contact::m_task_index,
        "hasLocation", &Contact::m_has_location,
        "location", &Contact::m_location,
        "confidantThreshold", &Contact::m_confidant_threshold,
        "friendThreshold", &Contact::m_friend_threshold,
        "completeThreshold", &Contact::m_complete_threshold,
        "canUseCell", &Contact::m_can_use_cell

    );
    m_private->m_lua.new_usertype<MapClientSession>( "MapClientSession",

        "new", sol::no_constructor, // The client links are not constructible from the script side.
        "m_ent",  sol::readonly( &MapClientSession::m_ent ),
        "admin_chat_message", sendChatMessage,
        "simple_dialog", [](MapClientSession *cl,const char *dlgtext)
        {
            auto n = new StandardDialogCmd(dlgtext);
            cl->addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(n));
        },
        "browser", [](MapClientSession *cl, const char *content){
            cl->addCommand<Browser>(content);
        },

        "contact_dialog",[](MapClientSession *cl, const char *message, sol::as_table_t<std::map<std::string, sol::as_table_t<std::vector<std::string>>>> buttons)
        {
            std::vector<ContactEntry> active_contacts;
            const auto& listMap = buttons.source;

            for (const auto& kvp : listMap)
            {
                const std::vector<std::string>& strings = kvp.second.source;
                int count = 0;
                ContactEntry con;
                for (const auto& s: strings){
                    if(count == 0)
                    {
                        con.m_response_text = QString::fromStdString(s);
                    }
                    else
                    {
                         con.m_link = contactLinkHash.find(QString::fromStdString(s)).value();
                    }
                    //sendInfoMessage(MessageChannel::ADMIN, QString::fromStdString(s) ,*cl);

                    count++;
                }
                active_contacts.push_back(con);
            }
            sendContactDialog(*cl, message, active_contacts);
        },
        "close_dialog", [](MapClientSession *cl){
            cl->addCommand<ContactDialogClose>();
        },
        "sendFloatingInfo",[](MapClientSession *cl, int message_type)
        {
            FloatingInfoMsgKey f_info_message = static_cast<FloatingInfoMsgKey>(message_type);
            QString message = FloatingInfoMsg.find(f_info_message).value();
            cl->addCommand<FloatingInfo>(cl->m_ent->m_idx, message, FloatingInfo_Attention , 4.0);
        },
        "sendInfoMessage", [](MapClientSession *cl, int channel, const char* message)
        {
            sendInfoMessage(static_cast<MessageChannel>(channel), QString::fromUtf8(message), *cl);
        },
        "addNpc", addNpc
        );


    m_private->m_lua.new_usertype<Character>("Character",
    "giveDebt", giveDebt,
    "giveEnhancement", giveEnhancement,
    "giveEnd", giveEnd,
    "giveHp", giveHp,
    "giveInf",giveInf,
    "giveInsp", giveInsp,
    "giveXp", giveXp,
    "sendFloatingDamage",sendFloatingNumbers,
    "faceEntity",sendFaceEntity,
    "faceLocation",  sendFaceLocation,
    "addUpdateContactList", [](MapClientSession *cl, Contact contact, glm::vec3 loc)
    {
        Destination des;
        des.location = loc;
        contact.m_location = des;
        sendContactStatusList(*cl, contact);
    }

    );

    m_private->m_lua.new_usertype<Entity>( "Entity",
        "new",    sol::no_constructor, // not constructible from the script side.
        sol::meta_function::garbage_collect, sol::destructor( destruction_is_an_error<Entity> ),
        "abort_logout",  abortLogout,
        "begin_logout",  &Entity::beginLogout
    );
    m_private->m_lua.new_usertype<MapSceneGraph>( "MapSceneGraph",
        "new", sol::no_constructor, // The client links are not constructible from the script side.
        "set_default_spawn_point", &MapSceneGraph::set_default_spawn_point
    );
    m_private->m_lua.script("function ErrorHandler(msg) return \"Lua call error:\"..msg end");
    m_private->m_lua["printDebug"] = [](const char* msg)
    {
        qCDebug(logScripts) << msg;
    };
}

int ScriptingEngine::loadAndRunFile(const QString &filename)
{
    sol::load_result load_res=m_private->m_lua.load_file(filename.toStdString());
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

std::string ScriptingEngine::callFuncWithClientContext(MapClientSession *client, const char *name, int arg1)
{
    m_private->m_lua["client"] = client;
    m_private->m_lua["vContacts"] = client->m_ent->m_char->m_char_data.m_contacts;
    m_private->m_lua["heroName"] = qPrintable(client->m_name);
    return callFunc(name,arg1);
}

std::string ScriptingEngine::callFuncWithClientContext(MapClientSession *client, const char *name, int arg1, glm::vec3 loc)
{
    m_private->m_lua["client"] = client;
    m_private->m_lua["vContacts"] = client->m_ent->m_char->m_char_data.m_contacts;
    m_private->m_lua["heroName"] = qPrintable(client->m_name);
    return callFunc(name,arg1,loc);
}

std::string ScriptingEngine::callFunc(const char *name, int arg1)
{
    sol::protected_function funcwrap = m_private->m_lua[name];
    funcwrap.error_handler = m_private->m_lua["ErrorHandler"];
    if(!funcwrap.valid())
    {
        qCritical() << "Failed to retrieve script func:"<<name;
        return "";
    }
    auto result = funcwrap(arg1);
    if(!result.valid())
    {
        sol::error err = result;
        qCritical() << "Failed to run script func:"<<name<<err.what();
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
    if (!load_res.valid())
    {
        sol::error err = load_res;
        // TODO: report error here.
        return -1;
    }
    sol::protected_function_result script_result = load_res();
    if (!script_result.valid())
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
}

//! @}
