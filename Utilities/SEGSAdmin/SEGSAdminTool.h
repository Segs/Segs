/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef SEGSADMINTOOL_H
#define SEGSADMINTOOL_H

#include "Version.h"
#include <QMainWindow>
#include <QDialog>
#include <QProcess>
#include <QFontDatabase>


namespace Ui {
class SEGSAdminTool;
}

class SEGSAdminTool : public QMainWindow
{
    Q_OBJECT
    class AddNewUserDialog *m_add_user_dialog;
    class GenerateConfigFileDialog *m_generate_config_dialog;
    class SetUpData *m_set_up_data;
    class SettingsDialog *m_settings_dialog;
    class NetworkManager *m_network_manager;
    class UpdateDetailDialog *m_update_dialog;
    class AboutDialog *m_about_dialog;
    class SelectScriptDialog *m_script_dialog;
    bool m_server_running = false;

public:
    explicit SEGSAdminTool(QWidget *parent = nullptr);
    ~SEGSAdminTool();

public slots:
    void commit_user(QString username, QString password, QString acclevel);
    void create_databases(bool overwrite);
    void check_db_exist(bool on_startup);
    void start_segs_server();
    void stop_segs_server();
    void read_createuser();
    void read_createDB();
    void read_segsserver();
    void check_for_config_file();
    void checkDataAndDir();
    void readReleaseInfo(const QString &error);
    void check_config_version(QString filePath);

signals:
    void readyToRead(QString filePath);
    void checkForConfigFile();
    void checkForDB(bool on_startup);
    void addAdminUser();
    void getMapsDirConfigCheck();
    void sendMapsDir(QString maps_dir);
    void newVersionAvailable(QString release_id);
    void getLatestReleases();
    void checkConfigVersion(QString filePath);
    void recreateConfig();


private:
    Ui::SEGSAdminTool *ui;
    void is_server_running();
    QProcess *m_createUser;
    QProcess *m_createDB;
    QProcess *m_start_segs_server;
    QStringList m_segs_releases;
};

#endif // SEGSADMINTOOL_H
