/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */


#include "ScriptingEngine.h"



void ScriptingEngine::registerGenericTypes()
{
    m_private->m_lua.new_usertype<QString>( "QString",
        sol::constructors<QString(), QString(const char*)>()
    );
}
