/*
 * Super Entity Game Server
 * http://github.com/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include <QSettings>
#include <QString>

class Settings {

public:
    static QSettings*   getSettings();
    static void         setSettingsPath(const QString path);
    static QString      getSettingsPath();

    void        createSettingsFile();
    void        setDefaultSettings();

private:
    Settings();
    Settings(Settings const&);
    Settings& operator=(Settings const&);

    static QSettings*   m_settings;
    static QString      m_settings_path;
};

void settingsDump();
void settingsDump(QSettings *s);
