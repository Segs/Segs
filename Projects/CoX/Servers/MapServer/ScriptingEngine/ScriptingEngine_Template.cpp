/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "ScriptingEngine.h"

#include <QtGlobal>

ScriptingEngine::ScriptingEngine() {}
ScriptingEngine::~ScriptingEngine() {}
void ScriptingEngine::registerTypes() {}
void ScriptingEngine::register_GenericTypes() {}
void ScriptingEngine::register_CharacterTypes() {}
void ScriptingEngine::register_SpawnerTypes() {}

void updateClientContext(MapClientSession * client)
{
    Q_UNUSED(client);
}

int ScriptingEngine::runScript(const QString &script_contents, const char *script_name)
{
    Q_UNUSED(script_contents);
    Q_UNUSED(script_name);
    return -1;
}

int ScriptingEngine::runScript(MapClientSession * client, const QString &script_contents, const char *script_name)
{
    Q_UNUSED(client);
    Q_UNUSED(script_contents);
    Q_UNUSED(script_name);
    return -1;
}

int ScriptingEngine::loadAndRunFile(const QString &path)
{
    Q_UNUSED(path);
    return -1;
}

bool ScriptingEngine::setIncludeDir(const QString &)
{
    return false;
}

std::string ScriptingEngine::callFunc(const char *name, int arg1)
{
    Q_UNUSED(name);
    Q_UNUSED(arg1);
    return "";
}

std::string ScriptingEngine::callFunc(const char *name, int arg1, glm::vec3 loc)
{
    Q_UNUSED(name);
    Q_UNUSED(arg1);
    Q_UNUSED(loc);
    return "";
}

std::string ScriptingEngine::callFunc(const char *name, const char *arg1, glm::vec3 loc)
{
    Q_UNUSED(name);
    Q_UNUSED(arg1);
    Q_UNUSED(loc);
    return "";
}

std::string ScriptingEngine::callFunc(const char *name, std::vector<Contact> contact_list)
{
    Q_UNUSED(name);
    Q_UNUSED(contact_list);
    return "";
}


std::string ScriptingEngine::callFuncWithClientContext(MapClientSession *client, const char *name, int arg1)
{
    Q_UNUSED(client);
    Q_UNUSED(name);
    Q_UNUSED(arg1);
    return "";
}

std::string ScriptingEngine::callFuncWithClientContext(MapClientSession *client, const char *name, int arg1, glm::vec3 loc)
{
    Q_UNUSED(client);
    Q_UNUSED(name);
    Q_UNUSED(arg1);
    Q_UNUSED(loc);
    return "";
}

std::string ScriptingEngine::callFuncWithClientContext(MapClientSession *client, const char *name, const char *arg1, glm::vec3 loc)
{
    Q_UNUSED(client);
    Q_UNUSED(name);
    Q_UNUSED(arg1);
    Q_UNUSED(loc);
    return "";
}

void ScriptingEngine::updateClientContext(MapClientSession *client)
{
    Q_UNUSED(client);
}

void ScriptingEngine::updateMapInstance(MapInstance *mi)
{
    Q_UNUSED(mi);
}

void ScriptingEngine::callFuncWithMapInstance(MapInstance *mi, const char *name, int arg1)
{
    Q_UNUSED(mi);
    Q_UNUSED(name);
    Q_UNUSED(arg1);
}

//! @}
