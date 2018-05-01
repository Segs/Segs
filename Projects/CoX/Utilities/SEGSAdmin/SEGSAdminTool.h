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

public:
    explicit SEGSAdminTool(QWidget *parent = 0);
    ~SEGSAdminTool();


public Q_SLOTS:
    void commit_user(const QString username, const QString password, const QString acclevel);
    void create_databases(bool overwrite);
    void check_db_exist();
    void start_auth_server();
    void stop_auth_server();
    void read_createuser();
    void read_createDB();
    void read_authserver();
    //void GetConfigFile();
    void read_config_file(const QString filePath);
    void check_for_config_file();

signals:
    void readyToRead(const QString filePath);


private:
    Ui::SEGSAdminTool *ui;
    QProcess *m_createUser;
    QProcess *m_createDB;
    QProcess *m_start_auth_server;
};

#endif // SEGSADMINTOOL_H
