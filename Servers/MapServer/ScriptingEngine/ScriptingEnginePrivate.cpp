/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "ScriptingEnginePrivate.h"
#include <QtCore/QFileInfo> // for include support
#include <QtCore/QDir>
#include <QtCore/QDebug>

ScriptingEnginePrivate::ScriptingEnginePrivate()
{
    m_lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::table, sol::lib::string, sol::lib::math,
                         sol::lib::utf8, sol::lib::debug);
}

ScriptingEnginePrivate::~ScriptingEnginePrivate() = default;


bool ScriptingEnginePrivate::performInclude(const char *path)
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
