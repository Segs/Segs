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
#include <QTextStream>#include <QFile>
#include <QFileInfo>
#include <QDebug>

class Settings {

public:
    Settings();

    static QSettings    *getSettings();
    void         createSettingsFile();
    void         setDefaultSettings();

    QString     m_settings_path = "settings.cfg"; // default path 'settings.cfg'
    QFile       m_settings_file;

private:
    static QSettings m_settings;
};

void settingsDump();
