/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef SEGSADMINTOOL_H
#define SEGSADMINTOOL_H

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
    bool m_server_running = false;


    //int font_output_id = QFontDatabase::addApplicationFont(":/fonts/dejavusanscondensed.ttf");
    //QString font_output_family = QFontDatabase::applicationFontFamilies(id).at(0);
    //QFont monospace(font_output_family);


    //const QFont fixedFont = QFontDatabase::addApplicationFont(":/fonts/dejavusanscondensed.ttf");

public:
    explicit SEGSAdminTool(QWidget *parent = nullptr);
    ~SEGSAdminTool();

public slots:
    void commit_user(QString username, QString password, QString acclevel);
    void create_databases(bool overwrite);
    void check_db_exist(bool on_startup);
    void start_auth_server();
    void stop_auth_server();
    void read_createuser();
    void read_createDB();
    void read_authserver();
    void check_for_config_file();
    void check_data_and_dir(QString maps_dir);

signals:
    void readyToRead(QString filePath);
    void checkForConfigFile();
    void checkForDB(bool on_startup);
    void addAdminUser();
    void getMapsDirConfigCheck();
    //void sendMapsDir(QString maps_dir);


private:
    Ui::SEGSAdminTool *ui;
    void is_server_running();
    QProcess *m_createUser;
    QProcess *m_createDB;
    QProcess *m_start_auth_server;

};

#endif // SEGSADMINTOOL_H
