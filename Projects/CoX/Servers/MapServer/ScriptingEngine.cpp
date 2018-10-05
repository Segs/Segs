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
#include <iomanip>
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
        "display_name", &Contact::m_display_name
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
            cl->addCommand<ContactDialog>(message, active_contacts);
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
        }
    , // Error on line 165 prevents this from working. Not sure how to handle
    "addNpc", [](MapClientSession &cl, const char* name, sol::as_table_t<std::vector<std::float_t>> location, int variation)
    {
        glm::vec3 gm_loc = cl.m_ent->m_entity_data.m_pos;
        const NPCStorage & npc_store(getGameData().getNPCDefinitions());
        const QString name_string = QString::fromUtf8(name);
        QStringRef npc_name = QStringRef(&name_string);

        const Parse_NPC * npc_def = npc_store.npc_by_name(npc_name);
        if(!npc_def)
        {
            sendInfoMessage(MessageChannel::USER_ERROR, "No NPC definition for:"+npc_name, cl);
            return;
        }
        int idx = npc_store.npc_idx(npc_def);
        Entity *e = cl.m_current_map->m_entities.CreateNpc(getGameData(),*npc_def,idx,variation);

        glm::vec3 loc;
        const auto& floats = location.source;
        int count = 0;
        for (const auto& f : floats)
        {
            if(count == 0)
            {
                loc.x = f;
            }
            else if (count == 1)
            {
                loc.y = f;
            }
            else
            {
                loc.z = f;
            }
            count++;
        }

        forcePosition(*e, loc);
        e->m_velocity = {0,0,0};
        sendInfoMessage(MessageChannel::DEBUG_INFO, QString("Created npc with ent idx:%1 at location x: %2 y: %3 z: %4").arg(e->m_idx).arg(loc.x).arg(loc.y).arg(loc.z), cl);
    }
        );

    m_private->m_lua.new_usertype<Character>("Character",
    "giveDebt", [](MapClientSession *cl, int debt)
    {
        uint32_t current_debt = getDebt(*cl->m_ent->m_char);
        uint32_t debt_to_give = current_debt + debt;
        setDebt(*cl->m_ent->m_char, debt_to_give);
        QString msg = "Setting Debt to " + QString::number(debt_to_give);
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *cl);
    },
    "giveEnhancement",[](MapClientSession *cl, sol::as_table_t<std::vector<std::string>> enhancement)
    {
        CharacterData &cd = cl->m_ent->m_char->m_char_data;
        QString name;
        uint32_t level;
        QString msg = "You do not have room for any more enhancements!";

        const auto& strings = enhancement.source;
        int count = 0;
        for (const auto& s : strings) {
            if(count == 0){
                name = QString::fromStdString(s);
            }else{
                level = QString::fromStdString(s).toInt();
            }
            count++;
        }

        if(getNumberEnhancements(cd) < 10)
        {
            msg = "Awarding Enhancement '" + name + "' to " + cl->m_ent->name();

            addEnhancementByName(cd, name, level);
            cd.m_has_updated_powers = true;

            QString floating_msg = FloatingInfoMsg.find(FloatingMsg_FoundEnhancement).value();
            sendFloatingInfo(*cl, floating_msg, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
        }

        qCDebug(logSlashCommand).noquote() << msg;
        sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *cl);
    },
    "giveEnd", [](MapClientSession *cl, float end)
    {
        float current_end = getEnd(*cl->m_ent->m_char);
        float end_to_set = current_end + end;
        setEnd(*cl->m_ent->m_char, end_to_set);
        QString msg = QString("Setting Endurance to: %1").arg(end_to_set);
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *cl);
    },

    "giveHp", [](MapClientSession *cl, float hp)
    {
        float current_hp = getHP(*cl->m_ent->m_char);
        float hp_to_set = current_hp + hp;
        setHP(*cl->m_ent->m_char, hp_to_set);
        QString msg = QString("Setting HP to: %1").arg(hp_to_set);
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *cl);
    },
    "giveInf", [](MapClientSession *cl, int inf)
    {
        uint32_t current_inf = getInf(*cl->m_ent->m_char);
        uint32_t inf_to_set = current_inf + inf;
        setInf(*cl->m_ent->m_char, inf_to_set);
        sendInfoMessage(MessageChannel::DEBUG_INFO, QString("Setting inf to %1").arg(inf_to_set), *cl);
    },
    "giveInsp", [](MapClientSession *cl, const char *value){

        CharacterData &cd = cl->m_ent->m_char->m_char_data;
        QString val = QString::fromUtf8(value);
        QString msg = "You do not have room for any more inspirations!";

        if(getNumberInspirations(cd) < getMaxNumberInspirations(cd))
        {
            msg = "Awarding Inspiration '" + val + "' to " + cl->m_ent->name();

            addInspirationByName(cd, val);
            cd.m_has_updated_powers = true;

            // NOTE: floating message shows no message here, but plays the awarding insp sound!
            QString floating_msg = FloatingInfoMsg.find(FloatingMsg_FoundInspiration).value();
            sendFloatingInfo(*cl, floating_msg, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
        }

        qCDebug(logSlashCommand).noquote() << msg;
        sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *cl);
    },
    "giveXp", [](MapClientSession *cl, int xp){
        uint32_t lvl = getLevel(*cl->m_ent->m_char);
        uint32_t current_xp = getXP(*cl->m_ent->m_char);

        // Calculate XP - Debt difference by server settings?

        uint32_t current_debt = getDebt(*cl->m_ent->m_char);
        if(current_debt > 0)
        {
            uint32_t debt_to_pay = 0;
            uint32_t half_xp = xp / 2;
            if(current_debt > half_xp)
            {
                debt_to_pay = half_xp; //Half to debt
                xp = half_xp;
            }
            else
            {
                debt_to_pay = current_debt;
                xp = xp - current_debt;
            }
            uint32_t newDebt = current_debt - debt_to_pay;
            setDebt(*cl->m_ent->m_char, newDebt);
            sendInfoMessage(MessageChannel::DEBUG_INFO, QString("You paid %1 to your debt").arg(debt_to_pay), *cl);
        }
        uint32_t xp_to_give = current_xp + xp;
        setXP(*cl->m_ent->m_char, xp_to_give);
        sendInfoMessage(MessageChannel::DEBUG_INFO, QString("You were awarded %1 XP").arg(xp), *cl);
        QString msg = "Setting XP to " + QString::number(xp_to_give);
        uint32_t new_lvl = getLevel(*cl->m_ent->m_char);
        sendInfoMessage(MessageChannel::DEBUG_INFO, QString("You were lvl %1 now %2").arg(lvl).arg(new_lvl), *cl);

        //This check doesn't show level change

        if(new_lvl != lvl)
        {
            cl->addCommand<FloatingInfo>(cl->m_ent->m_idx, FloatingInfoMsg.find(FloatingMsg_Leveled).value(), FloatingInfo_Attention , 4.0);
            msg += " and LVL to " + QString::number(new_lvl);
        }
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::DEBUG_INFO, msg, *cl);
    },
    "sendFloatingDamage", [](MapClientSession *cl, int target, int amount)
    {
         cl->addCommand<FloatingDamage>(cl->m_ent->m_idx, target, amount);
    },
    "faceEntity",[](MapClientSession *cl, int target)
    {
        sendFaceEntity(*cl->m_ent, target);
    },
    "faceLocation", [](MapClientSession *cl, sol::as_table_t<std::vector<std::float_t>> location)
    {
        glm::vec3 loc;
        const auto& floats = location.source;
        int count = 0;
        for (const auto& f : floats)
        {
            if(count == 0)
            {
                loc.x = f;
            }
            else if (count == 1)
            {
                loc.y = f;
            }
            else
            {
                loc.z = f;
            }
            count++;
        }

        qCDebug(logScripts) << QString("Facing location. X: %1 Y: %2 Z: %3").arg(loc.x).arg(loc.y).arg(loc.z);
        sendFaceLocation(*cl->m_ent, loc);
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
    m_private->m_lua["contactTable"] = m_private->m_lua.create_named_table("contactTable");
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
    m_private->m_lua["heroName"] = qPrintable(client->m_name);
    return callFunc(name,arg1);
}

std::string ScriptingEngine::callFuncWithClientContext(MapClientSession *client, const char *name, int arg1, glm::vec3 vec3)
{
    m_private->m_lua["client"] = client;
    m_private->m_lua["heroName"] = qPrintable(client->m_name);
    return callFunc(name,arg1,vec3);
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

std::string ScriptingEngine::callFunc(const char *name, int arg1, glm::vec3 vec3)
{
    sol::protected_function funcwrap = m_private->m_lua[name];
    funcwrap.error_handler = m_private->m_lua["ErrorHandler"];

    sol::table loc_table = m_private->m_lua.create_table_with("x", vec3.x, "y",  vec3.y, "z", vec3.z);
    sol::table table = m_private->m_lua.create_table_with("id", arg1, "loc" , loc_table);

    if(!funcwrap.valid())
    {
        qCritical() << "Failed to retrieve script func:"<<name;
        return "";
    }
    auto result = funcwrap(table);
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
