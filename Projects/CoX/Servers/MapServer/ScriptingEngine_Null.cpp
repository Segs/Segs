/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "ScriptingEngine.h"
ScriptingEngine::ScriptingEngine() {}
ScriptingEngine::~ScriptingEngine() {}
void ScriptingEngine::registerTypes() {}
int ScriptingEngine::runScript(const QString &script_contents, const char *script_name) {return -1;}
int ScriptingEngine::runScript(MapClientSession * client, const QString &script_contents, const char *script_name) {return -1;}
int ScriptingEngine::loadAndRunFile(const QString &path) { return -1; }
std::string ScriptingEngine::callFunc(const char *name, int arg1) { return ""; }
std::string ScriptingEngine::callFuncWithClientContext(MapClientSession *client, const char *name, int arg1) { return "";}

//! @}
