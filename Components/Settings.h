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
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

class Settings {

public:
    Settings();

    static QSettings    *getSettings();
    void                setSettingsPath(const QString path);
    QString             getSettingsPath();

    void        createSettingsFile();
    void        setDefaultSettings();

private:
    static QSettings m_settings;
    QString   m_settings_path; // default path 'settings.cfg' from args
};

void settingsDump();
void settingsDump(QSettings *s);
