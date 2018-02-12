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
#include <QFile>
#include <QFileInfo>
#include <QDebug>

class Settings : public QSettings {

public:
    Settings();
    ~Settings();

    QSettings  *getSettings();
    void        createSettingsFile();
    void        setDefaultSettings();
    void        dump();

    QString     m_settings_path = "settings.cfg"; // default path 'settings.cfg'
    QFile       m_settings_file;

private:
    QSettings* m_settings;
};
