/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup Components
 * @{
 */

#include "Settings.h"
#include "Logging.h"

#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QDir>

QString Settings::m_segs_dir;
QString Settings::m_settings_path = QStringLiteral("settings.cfg"); // default path 'settings.cfg' from args
QString Settings::m_default_tpl_dir = QStringLiteral("default_setup"); // default folder 'default_setup'
QString Settings::m_default_settings_path = Settings::m_default_tpl_dir + QDir::separator() + QStringLiteral("settings_template.cfg"); // default template from folder 'default_setup'

bool fileExists(const QString &path)
{
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and not a directory?
    return check_file.exists() && check_file.isFile() && check_file.size() > 0;
}

Settings::Settings()
{
    if(!fileExists(getSettingsPath()))
        createSettingsFile();
}

void Settings::setSettingsPath(const QString path)
{
    if(path.isEmpty())
        qCritical() << "Settings path not defined? This is unpossible!";

    m_settings_path = getSEGSDir() + QDir::separator() + path;
    qCDebug(logSettings) << "Settings Path" << m_settings_path;
}

QString Settings::getSettingsPath()
{
    if(m_settings_path.isEmpty())
        setSettingsPath("settings.cfg"); // set default path to "settings.cfg"

    return m_settings_path;
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

    if(!fileExists(curdir.absolutePath() + QDir::separator() + "segs_server"))
        qWarning() << "Cannot find SEGS Server!";

    m_segs_dir = curdir.absolutePath();
}

QString Settings::getSEGSDir()
{
    // if m_segs_dir is not empty, we've set it, return that instead
    if(m_segs_dir.isEmpty())
        setSEGSDir();

    return m_segs_dir;
}

QString Settings::getSettingsTplPath()
{
    QDir curdir(getSEGSDir()); // Get the SEGS working directory
    if(!fileExists(curdir.absolutePath() + QDir::separator() + m_default_settings_path))
        qWarning() << "Cannot find" << m_default_settings_path;

    return curdir.absolutePath() + QDir::separator() + m_default_settings_path;
}

QString Settings::getTemplateDirPath()
{
    QDir curdir(getSEGSDir()); // Get the SEGS working directory
    if(!QDir(curdir.absolutePath() + QDir::separator() + m_default_tpl_dir).exists())
        qWarning() << "Cannot find directory" << m_default_tpl_dir;

    return curdir.absolutePath() + QDir::separator() + m_default_tpl_dir;
}

void Settings::createSettingsFile()
{
    if(!fileExists(Settings::getSettingsPath()))
    {
        qCritical() << "Settings file" << Settings::getSettingsPath() <<"does not exist. Creating it now...";
        QFile sfile(Settings::getSettingsPath());
        if(!sfile.open(QIODevice::WriteOnly))
        {
            qDebug() << "Unable to create" << Settings::getSettingsPath() << "Check folder permissions.";
            return;
        }

        // QSettings setValue() methods delete all file comments, it's better to
        // simply copy the template over to our destination directory.
        QFile::copy(Settings::getSettingsTplPath(), Settings::getSettingsPath());

        return;
    }
    else
    {
        qDebug() << "Settings file already exists at" << Settings::getSettingsPath();
        return;
    }
}

void settingsDump()
{
    QSettings config(Settings::getSettingsPath(),QSettings::IniFormat,nullptr);
    settingsDump(&config);
}

void settingsDump(QSettings *s)
{
    QString output = "Settings File Dump\n";
    foreach (const QString &group, s->childGroups()) {
        QString groupString = QString("===== %1 =====\n").arg(group);
        s->beginGroup(group);

        foreach (const QString &key, s->allKeys()) {
            groupString.append(QString("  %1\t\t %2\n").arg(key, s->value(key).toString()));
        }

        s->endGroup();
        groupString.append("\n");

        output.append(groupString);
    }
    qDebug().noquote() << output;
}

//! @}
