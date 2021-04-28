/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/GameData/Contact.h"
#include "Common/GameData/Entity.h"
#include <array>
#include <memory>
#include <string>
#include <QtCore/QFileInfo> // for include support
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include "MapSceneGraph.h"

class QString;
struct MapClientSession;
class MapInstance;
class ScriptingEnginePrivate;

class ScriptingEngine
{
public:
    ScriptingEngine();
    ~ScriptingEngine();
    void registerTypes();
    void register_GenericTypes();
    void register_CharacterTypes();
    void register_SpawnerTypes();
    int loadAndRunFile(const QString &path);
    void callFuncWithMapInstance(MapInstance *mi, const char *name, int arg1);
    std::string callFuncWithClientContext(MapClientSession *client,const char *name,int arg1);
    std::string callFuncWithClientContext(MapClientSession *client,const char *name,int arg1, glm::vec3 loc);
    std::string callFuncWithClientContext(MapClientSession *client, const char *name, const char *arg1, glm::vec3 loc);
    std::string callFunc(const char *name,int arg1);
    std::string callFunc(const char *name,int arg1, glm::vec3 loc);
    std::string callFunc(const char *name, const char *arg1, glm::vec3 loc);
    std::string callFunc(const char *name, std::vector<Contact> contact_list);
    void updateMapInstance(MapInstance * instance);
    void updateClientContext(MapClientSession * client);
    int runScript(const QString &script_contents,const char *script_name="unnamed script");
    int runScript(MapClientSession *client,const QString &script_contents,const char *script_name="unnamed script");
    bool setIncludeDir(const QString &path);
private:
    std::unique_ptr<ScriptingEnginePrivate> m_private;

    MapInstance *mi;
    MapClientSession *cl;
    Entity *e;

};
