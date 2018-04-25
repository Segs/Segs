/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include <QSettings>
#include <QString>

class Settings {

public:
    static void         setSettingsPath(const QString path);
    static QString      getSettingsPath();

    void        createSettingsFile();
    void        setDefaultSettings();

private:
    Settings();
    Settings(Settings const&);
    Settings& operator=(Settings const&);

    static QString      m_settings_path;
};

void settingsDump();
void settingsDump(QSettings *s);
