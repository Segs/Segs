/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "ScriptingEngine.h"
#include "MapInstance.h"
#include "GameData/playerdata_definitions.h"
#include "DataHelpers.h"

#include <QtCore/QFileInfo> // for include support
#include <QtCore/QDir>
#include <QtCore/QDebug>

using namespace SEGSEvents;
//static constexpr const int MAX_INCLUDED_FILE_SIZE=1024*1024; // 1MB of lua code should be enough for anyone :P
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

ScriptingEngine::ScriptingEngine()
{
     m_private.reset(new ScriptingEnginePrivate);
}

ScriptingEngine::~ScriptingEngine() = default;


void ScriptingEngine::registerTypes()
{
    register_GenericTypes();
    register_SpawnerTypes();
    register_CharacterTypes();

}



//! @}
