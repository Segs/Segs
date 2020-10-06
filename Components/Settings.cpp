/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup Components
 * @{
 */

#include "Components/Settings.h"
#include "Components/Logging.h"

#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QRegularExpression>

QString Settings::s_segs_dir;
QString Settings::s_settings_path = QStringLiteral("settings.cfg"); // default path 'settings.cfg' from args
QString Settings::s_default_tpl_dir = QStringLiteral("default_setup"); // default folder 'default_setup'
QString Settings::s_default_settings_path = Settings::s_default_tpl_dir + QDir::separator() + QStringLiteral("settings_template.cfg"); // default template from folder 'default_setup'

bool fileExists(const QString &path)
{
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and not a directory?
    return check_file.exists() && check_file.isFile() && check_file.size() > 0;
}

Settings::Settings()
{
    if(!fileExists(getSettingsPath()))
        qCritical() << "Settings path not defined? This is unpossible!";
}

void Settings::setSettingsPath(const QString &path)
{
    if(path.isEmpty())
        qCritical() << "Settings path not defined? This is unpossible!";

    s_settings_path = getSEGSDir() + QDir::separator() + path;

    if(!fileExists(s_settings_path))
        createSettingsFile(s_settings_path);

    qCDebug(logSettings) << "Settings Path" << s_settings_path;
}

QString Settings::getSettingsPath()
{
    if(s_settings_path.isEmpty())
        setSettingsPath("settings.cfg"); // set default path to "settings.cfg"

    return s_settings_path;
}

void Settings::setSEGSDir()
{
    // Get the current SEGS directory. This library is shared
    // by dbtool and others, so make sure we're in the correct
    // working directory
    QDir curdir(QDir::current());
    qCDebug(logSettings) << "Current Active Dir" << curdir.absolutePath();

    // if called from utilities, move up one directory
    if(curdir.absolutePath().endsWith("utilities", Qt::CaseInsensitive))
    {
        curdir.cdUp();
        qCDebug(logSettings) << "Root Dir" << curdir.absolutePath();
    }

    if(-1 == curdir.entryList().indexOf(QRegularExpression("segs_server.*")))
        qWarning() << "Cannot find SEGS Server at" << curdir.absolutePath();

    s_segs_dir = curdir.absolutePath();
}

QString Settings::getSEGSDir()
{
    // if m_segs_dir is not empty, we've set it, return that instead
    if(s_segs_dir.isEmpty())
        setSEGSDir();

    return s_segs_dir;
}

QString Settings::getSettingsTplPath()
{
    QDir curdir(getSEGSDir()); // Get the SEGS working directory
    if(!fileExists(curdir.absolutePath() + QDir::separator() + s_default_settings_path))
        qWarning() << "Cannot find" << s_default_settings_path;

    return curdir.absolutePath() + QDir::separator() + s_default_settings_path;
}

QString Settings::getTemplateDirPath()
{
    QDir curdir(getSEGSDir()); // Get the SEGS working directory
    if(!QDir(curdir.absolutePath() + QDir::separator() + s_default_tpl_dir).exists())
        qWarning() << "Cannot find directory" << s_default_tpl_dir;

    return curdir.absolutePath() + QDir::separator() + s_default_tpl_dir;
}

void Settings::createSettingsFile(const QString &new_file_path)
{
    qCDebug(logSettings) << "Creating Settings file" << new_file_path;
    QFile tpl_file(Settings::getSettingsTplPath());
    QFile new_file(new_file_path);

    if(!tpl_file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Unable to read" << tpl_file.fileName() << "Check folder permissions.";
        return;
    }

    // QSettings setValue() methods delete all file comments, it's better to
    // simply copy the template over to our destination directory.
    // Unfortunately QFile::copy() has some sort of bug and doesn't work
    // so instead let's open the new file, and copy the contents from template
    if(!new_file.open(QIODevice::WriteOnly) || !new_file.write(tpl_file.readAll()))
    {
        qWarning() << "Unable to create" << new_file_path << "Check folder permissions.";
        return;
    }

    new_file.close();
    tpl_file.close();
}

void settingsDump()
{
    QSettings config(Settings::getSettingsPath(),QSettings::IniFormat,nullptr);
    settingsDump(&config);
}

void settingsDump(QSettings *s)
{
    QString output = "Settings File Dump\n";
    for(const QString &group : s->childGroups())
    {
        QString groupString = QString("===== %1 =====\n").arg(group);
        s->beginGroup(group);

        for(const QString &key : s->allKeys())
            groupString.append(QString("  %1\t\t %2\n").arg(key, s->value(key).toString()));

        s->endGroup();
        groupString.append("\n");

        output.append(groupString);
    }
    qDebug().noquote() << output;
}

//! @}
