/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <QSettings>
#include <QString>

class Settings
{
public:
    static void         setSettingsPath(const QString &path);
    static QString      getSettingsPath();
    static void         setSEGSDir();
    static QString      getSEGSDir();
    static QString      getSettingsTplPath();
    static QString      getTemplateDirPath();
    static void         createSettingsFile(const QString &new_file_path);

private:
    Settings();
    Settings(Settings const&);
    Settings& operator=(Settings const&);

    static QString      s_segs_dir;
    static QString      s_settings_path;
    static QString      s_default_tpl_dir;
    static QString      s_default_settings_path;
};

void settingsDump();
void settingsDump(QSettings *s);
bool fileExists(const QString &path);
