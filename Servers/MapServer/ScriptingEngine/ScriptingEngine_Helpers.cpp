/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "GameData/playerdata_definitions.h"
#include "Messages/Map/ChatMessage.h"
#include "DataHelpers.h"
#include "ScriptingEngine.h"
#include "ScriptingEnginePrivate.h"
#include "MapInstance.h"

int ScriptingEngine::loadAndRunFile(const QString &filename)
{
    sol::load_result load_res = m_private->m_lua.load_file(filename.toStdString());
    if(!load_res.valid())
    {
        sol::error err = load_res;
        qCDebug(logScripts) << err.what();
        return -1;
    }

    sol::protected_function_result script_result = load_res();
    if(!script_result.valid())
    {
        sol::error err = script_result;
        qCDebug(logScripts) << err.what();
        return -1;
    }

    return 0;
}

void ScriptingEngine::callFuncWithMapInstance(MapInstance *mi, const char *name, int arg1)
{
    this->mi = mi;
    m_private->m_lua["contactDialogButtons"] = contactLinkHash;

    callFunc(name,arg1);
}

std::string ScriptingEngine::callFuncWithClientContext(MapClientSession *client, const char *name, int arg1)
{
    this->cl = client;
    this->mi = client->m_current_map;
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
    this->cl = client;
    this->mi = client->m_current_map;
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
    this->cl = client;
    this->mi = client->m_current_map;
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
        qCDebug(logScripts) << "Failed to retrieve script func:" << name;
        return "";
    }
    auto result = funcwrap(arg1);
    if(!result.valid())
    {
        sol::error err = result;
        qCDebug(logScripts) << "Failed to run script func:" << name << err.what();
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
        qCDebug(logScripts) << "Failed to retrieve script func:" << name;
        return "";
    }
    auto result = funcwrap(arg1, loc);
    if(!result.valid())
    {
        sol::error err = result;
        qCDebug(logScripts) << "Failed to run script func:"<<name<<err.what();
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
        qCDebug(logScripts) << "Failed to retrieve script func:"<<name;
        return "";
    }
    auto result = funcwrap(arg1, loc);
    if(!result.valid())
    {
        sol::error err = result;
        qCDebug(logScripts) << "Failed to run script func:"<<name<<err.what();
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
        qCDebug(logScripts) << "Failed to retrieve script func:"<<name;
        return "";
    }
    auto result = funcwrap(contact_list);
    if(!result.valid())
    {
        sol::error err = result;
        qCDebug(logScripts) << "Failed to run script func:"<<name<<err.what();
        return "";
    }
     return result.get<std::string>();
}

int ScriptingEngine::runScript(MapClientSession * client, const QString &script_contents, const char *script_name)
{
    this->cl = client;
    this->mi = client->m_current_map;
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
        qCDebug(logScripts) << err.what();
        return -1;
    }
    sol::protected_function_result script_result = load_res();
    if(!script_result.valid())
    {
        sol::error err = script_result;
        qCDebug(logScripts) << err.what();
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
    this->mi = instance;
}

void ScriptingEngine::updateClientContext(MapClientSession * client)
{
    this->cl = client;
    this->mi = client->m_current_map;
    m_private->m_lua["vContacts"] = client->m_ent->m_player->m_contacts;
    m_private->m_lua["heroName"] = qPrintable(client->m_name);
    m_private->m_lua["m_db_id"] = client->m_ent->m_db_id;
    if(client->m_ent->m_player->m_tasks_entry_list.size() > 0)
    {
        m_private->m_lua["vTaskList"] = client->m_ent->m_player->m_tasks_entry_list[0].m_task_list;
    }
}
