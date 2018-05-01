/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#ifndef SEGSADMINTOOL_H
#define SEGSADMINTOOL_H

#include <QMainWindow>
#include <QDialog>
#include <QProcess>

namespace Ui {
class SEGSAdminTool;
}

class SEGSAdminTool : public QMainWindow
{
    Q_OBJECT
    class AddNewUserDialog *m_add_user_dialog;
    class GenerateConfigFileDialog *m_generate_config_dialog;

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
    void read_config_file(QString filePath);
    void check_for_config_file();
    void save_changes_config_file();

signals:
    void readyToRead(QString filePath);
    void checkForConfigFile();
    void checkForDB(bool on_startup);
    void addAdminUser();


public slots:
    void generate_default_config_file(QString server_name, QString ip);

private:
    Ui::SEGSAdminTool *ui;
    QProcess *m_createUser;
    QProcess *m_createDB;
    QProcess *m_start_auth_server;
};

#endif // SEGSADMINTOOL_H
