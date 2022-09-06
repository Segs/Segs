/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2022 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QSettings>

QFile m_dark_theme(":/styles/Resources/styles/dark-theme.css");
QFile m_light_theme(":/styles/Resources/styles/light-theme.css");

ThemeManager::ThemeManager(QObject *parent)
    : QObject{parent}
{
    ThemeManager::startup();
}

void ThemeManager::startup()
{
    QString user_pref_theme = this->getUserPref();
    if (!user_pref_theme.isEmpty())
    {
        if (user_pref_theme == "dark")
        {
            this->setDarkTheme();
        }
        else if (user_pref_theme == "light")
        {
            this->setLightTheme();
        }
    }
    else
    {
        this->setLightTheme();
    }
}

void ThemeManager::setTheme(QFile &theme)
{
    theme.open(QFile::ReadOnly);
    QString style_sheet = QLatin1String(theme.readAll());
    theme.close();

    qApp->setStyleSheet(style_sheet);
}

// TODO: Save theme selection using QSettings
// TODO: Move theme setting to MenuBar
void ThemeManager::setDarkTheme()
{
    this->setTheme(m_dark_theme);
    this->setUserPref("dark");
}

void ThemeManager::setLightTheme()
{
    this->setTheme(m_light_theme);
    this->setUserPref("light");
}

QString ThemeManager::getUserPref()
{
    QSettings settings;
    QString theme = settings.value("user_theme").toString();

    return theme;
}

void ThemeManager::setUserPref(QString theme)
{
    QSettings settings;
    settings.setValue("user_theme", theme);
}


