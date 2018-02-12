/*
 * Super Entity Game Server
 * http://github.com/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
 
#include "Settings.h"

bool fileExists(QString path) {
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}

Settings::Settings()
{
    if (!fileExists(m_settings_path))
        createSettingsFile();

    if(!m_settings)
        QSettings m_settings(m_settings_path,QSettings::IniFormat);
}

QSettings *Settings::getSettings()
{
    if (!m_settings)
        m_settings =  new Settings();

    return m_settings;
}

void Settings::createSettingsFile()
{
    if (!fileExists(m_settings_path))
    {
        qCritical() << "Settings file" << m_settings_path <<"does not exist. Creating it now...";
        QFile m_settings_file(m_settings_path);
        if(!m_settings_file.open(QIODevice::WriteOnly))
        {
            qDebug() << "Unable to create" << m_settings_path << "Check folder permissions.";
            return;
        }
        m_settings_file.close();
        
        if(!m_settings)
            QSettings m_settings(m_settings_path,QSettings::IniFormat);
        
        setDefaultSettings();
        
        return;
    }
    else
    {
        qDebug() << "Settings file already exists at" << m_settings_path;
        return;
    }
}

void Settings::setDefaultSettings()
{
    m_settings->beginGroup("AdminServer");
        m_settings->beginGroup("AccountDatabase");
            m_settings->setValue("db_driver","QSQLITE");
            m_settings->value("db_host","127.0.0.1").toString();
            m_settings->value("db_port","5432").toString();
            m_settings->value("db_name","segs").toString();
            m_settings->value("db_user","none").toString();
            m_settings->value("db_pass","none").toString();
        m_settings->endGroup();
        m_settings->beginGroup("CharacterDatabase");
            m_settings->setValue("db_driver","QSQLITE");
            m_settings->value("db_host","127.0.0.1").toString();
            m_settings->value("db_port","5432").toString();
            m_settings->value("db_name","segs_game").toString();
            m_settings->value("db_user","none").toString();
            m_settings->value("db_pass","none").toString();
        m_settings->endGroup();
    m_settings->endGroup();
    m_settings->beginGroup("AuthServer");
        m_settings->value("listen_addr","127.0.0.1:2106").toString();
    m_settings->endGroup();
    m_settings->beginGroup("GameServer");
        m_settings->value("server_name","SEGS Server").toString();
        m_settings->value("listen_addr","127.0.0.1:7002").toString();
        m_settings->value("location_addr","127.0.0.1:7002").toString();
        m_settings->value("max_players","200").toInt();
        m_settings->value("max_account_slots","8").toInt();
    m_settings->endGroup();
    m_settings->beginGroup("MapServer");
        m_settings->value("listen_addr","127.0.0.1:7003").toString();
        m_settings->value("location_addr","127.0.0.1:7003").toString();
    m_settings->endGroup();
    
    m_settings->sync(); // sync changes
}

void Settings::dump()
{
    QString output;
    QSettings* settings = getSettings();
    foreach (const QString &group, settings->childGroups()) {
        QString groupString = QString("===== %1 =====\n").arg(group);
        settings->beginGroup(group);
    
        foreach (const QString &key, settings->childKeys()) {
            groupString.append(QString("  %1\t %2\n").arg(key, settings->value(key).toString()));
        }
        
        settings->endGroup();
        groupString.append("\n");
    
        output.append(groupString);
    }
    qDebug() << output;
}
