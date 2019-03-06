/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <QString>
#include <QSet>
#include <memory>
#include <string>
#include <lua/lua.hpp>
#include <sol2/sol.hpp>

class ScriptingEnginePrivate
{
    static constexpr const int MAX_INCLUDED_FILE_SIZE=1024*1024; // 1MB of lua code should be enough for anyone :P

public:
    sol::state          m_lua;
    QString             m_restricted_include_dir;
    QSet<QString>       m_alread_included_and_ran;

    ScriptingEnginePrivate();
    ~ScriptingEnginePrivate();
    bool performInclude(const char *path);

};
