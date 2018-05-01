/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup SEGSAdmin Projects/CoX/Utilities/SEGSAdmin
 * @{
 */

#include "SEGSAdminTool.h"
#include "ui_SEGSAdminTool.h"
#include "AddNewUserDialog.h"
#include <QDebug>
#include <QtGlobal>
#include <QProcess>
#include <QFileInfo>
#include <QFile>
#include <QFileDialog>
#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <iostream>

SEGSAdminTool::SEGSAdminTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SEGSAdminTool)

{
    ui->setupUi(this);
    ui->output->appendPlainText("*** Welcome to SEGSAdmin ***");
    ui->authserver_stop->setEnabled(false);
    m_add_user_dialog = new AddNewUserDialog;
    connect(m_add_user_dialog,&AddNewUserDialog::sendInput,this,&SEGSAdminTool::commit_user);
    connect(ui->createUser,&QPushButton::clicked,m_add_user_dialog,&AddNewUserDialog::on_add_user);
    connect(ui->runDBTool,&QPushButton::clicked,this,&SEGSAdminTool::check_db_exist);
    connect(ui->authserver_start,&QPushButton::clicked,this,&SEGSAdminTool::start_auth_server);
    connect(ui->authserver_stop,&QPushButton::clicked,this,&SEGSAdminTool::stop_auth_server);
    //connect(ui->selectfile,&QToolButton::clicked,this,&SEGSAdminTool::GetConfigFile);
    connect(this,&SEGSAdminTool::readyToRead,this,&SEGSAdminTool::read_config_file);
    SEGSAdminTool::check_for_config_file();


}

SEGSAdminTool::~SEGSAdminTool()
{
    delete ui;
}

void SEGSAdminTool::commit_user(const QString username, const QString password, const QString acclevel)
// TODO: Error checking & validate blank fields on save
{
    ui->output->appendPlainText("Setting arguments...");
    ui->createUser->setEnabled(false);
    ui->createUser->setText("Please Wait...");
    qApp->processEvents();
    qDebug() << "Setting arguments...";
    QString dbcommand = "dbtool adduser -l " + username + " -p " + password + " -a " + acclevel;
    #ifdef Q_OS_WIN
    QString program = "cmd.exe";
    QStringList arguments;
    arguments <<"/c" << dbcommand;
    qDebug() << "Windows detected...";
    ui->output->appendPlainText("Windows detected");
    #endif
    #ifdef Q_OS_LINUX
    QString program = "xterm";
    dbcommand.prepend("./");
    QStringList arguments;
    arguments <<"-e"<<dbcommand;
    ui->output->appendPlainText("Linux detected");
    #endif
    #ifdef Q_OS_MACOS // Adding this to satisfy Travis-CI build, not tested
    QString program = "/bin/sh";
    dbcommand.prepend("./");
    QStringList arguments;
    arguments <<"-e"<<dbcommand;
    ui->output->appendPlainText("MacOS detected");
    #endif

    m_createUser = new QProcess(this);
    m_createUser->start(program, arguments);

    if (m_createUser->waitForStarted())
    {
        ui->output->appendPlainText("Starting DBTool Add User...");
        qDebug() << "Starting DBTool Add User...";
        connect(m_createUser,&QProcess::readyReadStandardError,this,&SEGSAdminTool::read_createuser);
        connect(m_createUser,&QProcess::readyReadStandardOutput,this,&SEGSAdminTool::read_createuser);
        m_createUser->write("\n");
        m_createUser->closeWriteChannel();
        m_createUser->waitForFinished();
        ui->createUser->setText("Create User");
        ui->createUser->setEnabled(true);
        qApp->processEvents();
    }
    else
    {
        ui->output->appendPlainText("Failed to start DBTool Add User...");
        qDebug() <<"Failed to start DBTool Add User...";
    }
}

void SEGSAdminTool::read_createuser() // Not currently working with xterm/gnome-terminal
{
    QByteArray out_err = m_createUser->readAllStandardError();
    QByteArray out_std = m_createUser->readAllStandardOutput();
    qDebug().noquote()<<QString(out_err);
    qDebug().noquote()<<QString(out_std);
    QString output_err = out_err;
    QString output_std = out_std;
    output_err.replace("Press ENTER to continue...", "** FINISHED **");
    output_std.replace("Press ENTER to continue...", "** FINISHED **");
    ui->output->appendPlainText(output_err);
    ui->output->appendPlainText(output_std);
}

void SEGSAdminTool::check_db_exist()
{
    ui->output->appendPlainText("Checking for existing databases...");
    qDebug() << "Checking for existing databases...";
    QFileInfo m_file1("segs");
    QFileInfo m_file2("segs_game");

    if (m_file1.exists() || m_file2.exists())
    {
        QMessageBox db_overwrite_msgBox;
        db_overwrite_msgBox.setText("Confirm?");
        db_overwrite_msgBox.setInformativeText("Existing databases found, do you want to overwrite?");
        db_overwrite_msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        db_overwrite_msgBox.setDefaultButton(QMessageBox::No);
        int confirm = db_overwrite_msgBox.exec();
        switch (confirm) {
          case QMessageBox::Yes:
              SEGSAdminTool::create_databases(true);
              break;
          case QMessageBox::No:
              break;
          default:
              break;
        }
    }
    else
    {
        SEGSAdminTool::create_databases(false);
    }
}

void SEGSAdminTool::create_databases(bool overwrite)
{
    ui->runDBTool->setEnabled(false);
    ui->runDBTool->setText("Please Wait...");
    ui->output->appendPlainText("Setting arguments...");
    qApp->processEvents();
    qDebug() << "Setting arguments...";
    QString dbcommand = "dbtool create";
    if (overwrite)
    {
        dbcommand = dbcommand + " -f";
    }
    #ifdef Q_OS_WIN
    QString program = "cmd.exe";
    QStringList arguments;
    arguments <<"/c" << dbcommand;
    qDebug() << "Windows detected...";
    ui->output->appendPlainText("Windows detected");
    qApp->processEvents();
    #endif
    #ifdef Q_OS_LINUX
    QString program = "xterm";
    dbcommand.prepend("./");
    QStringList arguments;
    arguments <<"-e"<<dbcommand;
    ui->output->appendPlainText("Linux detected");
    qApp->processEvents();
    #endif
    #ifdef Q_OS_MACOS // Adding this to satisfy Travis-CI build, not tested
    QString program = "/bin/sh";
    dbcommand.prepend("./");
    QStringList arguments;
    arguments <<"-e"<<dbcommand;
    ui->output->appendPlainText("MacOS detected");
    #endif

    m_createDB = new QProcess(this);
    m_createDB->start(program, arguments);

    if (m_createDB->waitForStarted())
    {
        ui->output->appendPlainText("Starting DBTool Create Databases...");
        qApp->processEvents();
        qDebug() << "Starting DBTool Create Databases...";
        connect(m_createDB,&QProcess::readyReadStandardError,this,&SEGSAdminTool::read_createDB);
        connect(m_createDB,&QProcess::readyReadStandardOutput,this,&SEGSAdminTool::read_createDB);
        m_createDB->write("\n");
        m_createDB->closeWriteChannel();
        m_createDB->waitForFinished();
        ui->runDBTool->setText("Create Databases");
        ui->runDBTool->setEnabled(true);
        qApp->processEvents();
    }
    else
    {
        ui->output->appendPlainText("Failed to start DBTool Add User...");
        qDebug() <<"Failed to start DBTool Add User...";
        ui->runDBTool->setText("Create Databases");
        ui->runDBTool->setEnabled(true);
        qApp->processEvents();
    }
}

void SEGSAdminTool::read_createDB() // Not currently working with xterm/gnome-terminal
{
    QByteArray out_err = m_createDB->readAllStandardError();
    QByteArray out_std = m_createDB->readAllStandardOutput();
    qDebug().noquote()<<QString(out_err);
    qDebug().noquote()<<QString(out_std);
    QString output_err = out_err;
    QString output_std = out_std;
    output_err.replace("Press ENTER to continue...", "** FINISHED **");
    output_std.replace("Press ENTER to continue...", "** FINISHED **");
    ui->output->appendPlainText(output_err);
    ui->output->appendPlainText(output_std);
    qApp->processEvents();
}

void SEGSAdminTool::start_auth_server()
{
    ui->authserver_start->setEnabled(false);
    ui->authserver_start->setText("Please Wait...");
    ui->output->appendPlainText("Setting arguments...");
    qApp->processEvents();
    QString program = "authserver.exe";
    #ifdef Q_OS_LINUX
    program.prepend("./");
    #endif
    #ifdef Q_OS_MACOS
    program.prepend("./");
    #endif

    m_start_auth_server = new QProcess(this);
    m_start_auth_server->start(program);

    if (m_start_auth_server->waitForStarted())
    {
        ui->output->appendPlainText("Starting AuthServer...");
        ui->authserver_status->setText("STARTING");
        ui->authserver_status->setStyleSheet("QLabel {color: rgb(255, 153, 51)}");
        qApp->processEvents();
        qDebug() << "Starting AuthServer...";
        connect(m_start_auth_server,&QProcess::readyReadStandardError,this,&SEGSAdminTool::read_authserver);
        connect(m_start_auth_server,&QProcess::readyReadStandardOutput,this,&SEGSAdminTool::read_authserver);
        ui->authserver_start->setText("Start Auth Server");
        ui->authserver_stop->setEnabled(true);
        qApp->processEvents();
        m_start_auth_server->waitForFinished(2000);
        if(m_start_auth_server->state()==QProcess::Running)
        {
            ui->output->appendPlainText("*** AuthServer Running ***");
            ui->authserver_status->setText("RUNNING");
            ui->authserver_status->setStyleSheet("QLabel {color: rgb(0, 200, 0)}");
            ui->tabWidget->setEnabled(false);
            int pid = m_start_auth_server->processId();
            qDebug()<<pid;
        }
        if(m_start_auth_server->state()==QProcess::NotRunning)
        {
            ui->output->appendPlainText("*** AUTHSERVER NOT RUNNING... Have you setup your piggs and settings files? ***");
            ui->authserver_start->setText("Start Auth Server");
            ui->authserver_start->setEnabled(true);
            ui->authserver_status->setText("STOPPED");
            ui->authserver_status->setStyleSheet("QLabel {color: rgb(255, 0, 0)}");
        }

    }
    else
    {
        ui->output->appendPlainText("Failed to start AuthServer...");
        qDebug() <<"Failed to start AuthServer...";
        ui->authserver_start->setText("Start Auth Server");
        ui->authserver_start->setEnabled(true);
        qApp->processEvents();
    }

}

// Not currently working with xterm/gnome-terminal
// Some problem with reading large output
void SEGSAdminTool::read_authserver()
{
    QByteArray out_err = m_start_auth_server->readAllStandardError();
    QByteArray out_std = m_start_auth_server->readAllStandardOutput();
    qDebug().noquote()<<QString(out_err);
    qDebug().noquote()<<QString(out_std);
    QString output_err = out_err;
    QString output_std = out_std;
    output_err.replace("Press ENTER to continue...", "** FINISHED **");
    output_std.replace("Press ENTER to continue...", "** FINISHED **");
    ui->output->appendPlainText(output_err);
    ui->output->appendPlainText(output_std);
    qApp->processEvents();
}

void SEGSAdminTool::stop_auth_server()
{
    m_start_auth_server->close();
    if(m_start_auth_server->state()==QProcess::Running)
    {
        ui->output->appendPlainText("*** AuthServer Failed to Stop ***");
    }
    if(m_start_auth_server->state()==QProcess::NotRunning)
    {
        ui->authserver_start->setEnabled(true);
        ui->authserver_stop->setEnabled(false);
        ui->authserver_status->setText("STOPPED");
        ui->authserver_status->setStyleSheet("QLabel {color: rgb(255, 0, 0)}");
        ui->tabWidget->setEnabled(true);
        ui->output->appendPlainText("*** AuthServer Stopped ***");
    }
}

void SEGSAdminTool::check_for_config_file() // Does this on application start
{
    // Load settings.cfg if exists
    QFileInfo m_settings_cfg("settings.cfg");
    if (m_settings_cfg.exists())
    {
        QString m_configFile = m_settings_cfg.absoluteFilePath();
        ui->displayfile->setText(m_configFile);
        emit readyToRead(m_configFile);
    }
    else
    {
        ui->displayfile->setText("Unable to locate settings.cfg");
        ui->output->appendPlainText("** No settings.cfg file found! **");
    }

    // TODO: Ability to select new file location

    //QString configFile = QFileDialog::getOpenFileName(this,
    //tr("Select Config File"), "",
    //tr("Config Files (*.cfg);;All Files (*)"));
    //ui->displayfile->setText(configFile);
}

void SEGSAdminTool::read_config_file(const QString m_filePath) // May be a more elegant way to do this, but need to map each key to a UI field
{
    QSettings m_configfile(m_filePath, QSettings::IniFormat);
    m_configfile.beginGroup("AdminServer");
    m_configfile.beginGroup("AccountDatabase");
    QString acc_db_driver = m_configfile.value("db_driver","").toString();
    QString acc_db_host = m_configfile.value("db_host","").toString();
    QString acc_db_port = m_configfile.value("db_port","").toString();
    QString acc_db_name = m_configfile.value("db_name","").toString();
    QString acc_db_user = m_configfile.value("db_user","").toString();
    QString acc_db_pass = m_configfile.value("db_pass","").toString();
    ui->acc_dbdriver->setText(acc_db_driver);
    ui->acc_dbhost->setText(acc_db_host);
    ui->acc_dbport->setText(acc_db_port);
    ui->acc_dbname->setText(acc_db_name);
    ui->acc_dbuser->setText(acc_db_user);
    ui->acc_dbpass->setText(acc_db_pass);
    m_configfile.endGroup();
    m_configfile.beginGroup("CharacterDatabase");
    QString char_db_driver = m_configfile.value("db_driver","").toString();
    QString char_db_host = m_configfile.value("db_host","").toString();
    QString char_db_port = m_configfile.value("db_port","").toString();
    QString char_db_name = m_configfile.value("db_name","").toString();
    QString char_db_user = m_configfile.value("db_user","").toString();
    QString char_db_pass = m_configfile.value("db_pass","").toString();
    ui->char_dbdriver->setText(char_db_driver);
    ui->char_dbhost->setText(char_db_host);
    ui->char_dbport->setText(char_db_port);
    ui->char_dbname->setText(char_db_name);
    ui->char_dbuser->setText(char_db_user);
    ui->char_dbpass->setText(char_db_pass);
    m_configfile.endGroup();
    m_configfile.endGroup();
    m_configfile.beginGroup("AuthServer");
    QString auth_loc_addr = m_configfile.value("location_addr","").toString();
    QStringList auth_portip = auth_loc_addr.split(':');
    ui->auth_ip->setText(auth_portip[0]);
    ui->auth_port->setText(auth_portip[1]);
    m_configfile.endGroup();
    m_configfile.beginGroup("GameServer");
    QString game_server_name = m_configfile.value("server_name","").toString();
    QString game_listen_addr = m_configfile.value("listen_addr","").toString();
    QStringList game_listen_addr_portip = game_listen_addr.split(':');
    QString game_loc_addr = m_configfile.value("location_addr","").toString();
    QStringList game_loc_addr_portip = game_loc_addr.split(':');
    QString max_players = m_configfile.value("max_players","").toString();
    QString max_char_slots = m_configfile.value("max_character_slots","").toString();
    ui->game_server_name->setText(game_server_name);
    ui->game_listen_ip->setText(game_listen_addr_portip[0]);
    ui->game_listen_port->setText(game_listen_addr_portip[1]);
    ui->game_loc_ip->setText(game_loc_addr_portip[0]);
    ui->game_loc_port->setText(game_loc_addr_portip[1]);
    ui->game_max_players->setText(max_players);
    ui->game_max_slots->setText(max_char_slots);
    m_configfile.endGroup();
    m_configfile.beginGroup("MapServer");
    QString map_listen_addr = m_configfile.value("listen_addr","").toString();
    QStringList map_listen_addr_portip = map_listen_addr.split(':');
    QString map_loc_addr = m_configfile.value("location_addr","").toString();
    QStringList map_loc_addr_portip = map_loc_addr.split(':');
    QString maps_loc = m_configfile.value("maps","").toString();
    ui->map_listen_ip->setText(map_listen_addr_portip[0]);
    ui->map_listen_port->setText(map_listen_addr_portip[1]);
    ui->map_location_ip->setText(map_loc_addr_portip[0]);
    ui->map_location_port->setText(map_loc_addr_portip[1]);
    ui->map_location->setText(maps_loc);
    m_configfile.endGroup();
}

//!@}
