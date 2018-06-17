/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup SEGSAdmin Projects/CoX/Utilities/SEGSAdmin
 * @{
 */

#include "SEGSAdminTool.h"
#include "ui_SEGSAdminTool.h"
#include "AddNewUserDialog.h"
#include "GenerateConfigFileDialog.h"
#include "SetUpData.h"
#include "Globals.h"
#include "GetIPDialog.h"
#include "SettingsDialog.h"
#include <QDebug>
#include <QtGlobal>
#include <QProcess>
#include <QFileInfo>
#include <QFile>
#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <iostream>

SEGSAdminTool::SEGSAdminTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SEGSAdminTool)
{
    ui->setupUi(this);
    QFont dejavu_font;
    dejavu_font.setFamily("DejaVu Sans Condensed");
    dejavu_font.setPointSize(12);
    ui->output->setFont(dejavu_font);
    ui->output->appendPlainText("*** Welcome to SEGSAdmin ***");
    m_add_user_dialog = new AddNewUserDialog(this);
    m_set_up_data = new SetUpData(this);
    m_settings_dialog = new SettingsDialog(this);
    m_generate_config_dialog = new GenerateConfigFileDialog(this);

    // SEGSAdminTool Signals
    connect(this,&SEGSAdminTool::checkForDB,this,&SEGSAdminTool::check_db_exist);
    connect(this,&SEGSAdminTool::addAdminUser,m_add_user_dialog,&AddNewUserDialog::on_add_admin_user);
    connect(this,&SEGSAdminTool::checkForConfigFile,this,&SEGSAdminTool::check_for_config_file);
    connect(this,&SEGSAdminTool::getMapsDirConfigCheck,m_settings_dialog,&SettingsDialog::send_maps_dir_config_check); // May be a much better way to do this, but this works for now
    connect(this,&SEGSAdminTool::readyToRead,m_settings_dialog,&SettingsDialog::read_config_file);
    connect(ui->createUser,&QPushButton::clicked,m_add_user_dialog,&AddNewUserDialog::on_add_user);
    connect(ui->runDBTool,&QPushButton::clicked,this,&SEGSAdminTool::check_db_exist);
    connect(ui->set_up_data_button,&QPushButton::clicked,m_set_up_data,&SetUpData::open_data_dialog);
    connect(ui->settings_button,&QPushButton::clicked,m_settings_dialog,&SettingsDialog::open_settings_dialog);
    connect(ui->gen_config_file,&QPushButton::clicked,m_generate_config_dialog,&GenerateConfigFileDialog::on_generate_config_file);
    connect(ui->authserver_start,&QPushButton::clicked,this,&SEGSAdminTool::is_server_running);

    // GenerateConfigFileDialog Signals
    connect(m_generate_config_dialog,&GenerateConfigFileDialog::sendInputConfigFile,m_settings_dialog,&SettingsDialog::generate_default_config_file);

    // AddNewUserDialog Signals
    connect(m_add_user_dialog,&AddNewUserDialog::sendInput,this,&SEGSAdminTool::commit_user);

    // SetUpData Signals
    connect(m_set_up_data,&SetUpData::dataSetupComplete,this,&SEGSAdminTool::check_data_and_dir);
    connect(m_set_up_data,&SetUpData::getMapsDir,m_settings_dialog,&SettingsDialog::send_maps_dir);

    // SettingsDialog Signals
    connect(m_settings_dialog,&SettingsDialog::checkForConfigFile,this,&SEGSAdminTool::check_for_config_file);
    connect(m_settings_dialog,&SettingsDialog::check_data_and_dir,this,&SEGSAdminTool::check_data_and_dir);
    connect(m_settings_dialog,&SettingsDialog::sendMapsDirConfigCheck,this,&SEGSAdminTool::check_data_and_dir);
    connect(m_settings_dialog,&SettingsDialog::sendMapsDir,m_set_up_data,&SetUpData::create_default_directory);

    // Send startup signals
    emit checkForConfigFile();
    emit check_db_exist(true);
    emit getMapsDirConfigCheck();
}

SEGSAdminTool::~SEGSAdminTool()
{
    delete ui;
}

void SEGSAdminTool::check_data_and_dir(QString maps_dir) // Checks for data sub dirs and maps dir
{
    ui->output->appendPlainText("Checking for correct data and maps directories...");
    QStringList data_dirs = {"data/bin","data/geobin","data/object_library"}; // Currently only checking for these 3 sub dirs, check for all files could be overkill
    bool all_maps_exist = true;
    for(const QString &map_name : g_map_names)
    {
        all_maps_exist &= QDir(maps_dir+"/"+map_name).exists();
        if(!all_maps_exist)
            break; // at least one map dir is missing, we can finish early
    }
    bool all_data_dirs_exist = true;
    for(const QString &data_dir : data_dirs)
    {
        all_data_dirs_exist &= QDir(data_dir).exists();
        if(!all_data_dirs_exist)
            break; // at least one map dir is missing, we can finish early
    }
    if(all_data_dirs_exist && all_maps_exist)
    {
        ui->icon_status_data->setText("<html><head/><body><p><img src=':/icons/icon_good.png'/></p></body></html>");
        ui->output->appendPlainText("SUCCESS: Data and maps directories found!");
    }
    else
    {
        ui->icon_status_data->setText("<html><head/><body><p><img src=':/icons/icon_warning.png'/></p></body></html>");
        ui->output->appendPlainText("WARNING: We couldn't find the correct data and/or maps directories. Please use the server setup to your left");
    }

}

void SEGSAdminTool::commit_user(QString username, QString password, QString acclevel)
// TODO: Error checking & validate blank fields on save
{
    ui->output->appendPlainText("Setting arguments...");
    ui->createUser->setEnabled(false);
    ui->createUser->setText("Please Wait...");
    qApp->processEvents();
    qDebug() << "Setting arguments...";
    QString program = "dbtool adduser -l " + username + " -p " + password + " -a " + acclevel;
    #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    program.prepend("./");
    #endif
    m_createUser = new QProcess(this);
    m_createUser->start(program);

    if (m_createUser->waitForStarted())
    {
        ui->output->appendPlainText("Starting DBTool Add User...");
        qDebug() << "Starting DBTool Add User...";
        connect(m_createUser,&QProcess::readyReadStandardError,this,&SEGSAdminTool::read_createuser);
        connect(m_createUser,&QProcess::readyReadStandardOutput,this,&SEGSAdminTool::read_createuser);
        m_createUser->write("\n");
        m_createUser->closeWriteChannel();
        m_createUser->waitForFinished();
        ui->createUser->setText("Add New User");
        ui->createUser->setEnabled(true);
        qApp->processEvents();
    }
    else
    {
        ui->output->appendPlainText("Failed to start DBTool Add User...");
        qDebug() <<"Failed to start DBTool Add User...";
    }
}

void SEGSAdminTool::read_createuser()
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

void SEGSAdminTool::check_db_exist(bool on_startup)
{
    ui->output->appendPlainText("Checking for existing databases...");
    qDebug() << "Checking for existing databases...";
    QFileInfo file1("segs");
    QFileInfo file2("segs_game");
    if (on_startup) // Runs this check on startup or for checking creation in other methods
    {
        if (file1.exists() && file2.exists())
        {
            ui->output->appendPlainText("SUCCESS: Existing databases found!");
            ui->icon_status_db->setText("<html><head/><body><p><img src=':/icons/icon_good.png'/></p></body></html>");
            ui->runDBTool->setText("Create New Databases");
            ui->runDBTool->setEnabled(true);
            ui->createUser->setEnabled(true);
        }
        else
        {
            ui->output->appendPlainText("WARNING: Not all databases were found. Please use the server setup to your left");
            ui->icon_status_db->setText("<html><head/><body><p><img src=':/icons/icon_warning.png'/></p></body></html>");
            ui->createUser->setEnabled(false); // Cannot create users until DB's created
        }
    }
    else
    {
        if (file1.exists() || file2.exists())
        {
            QMessageBox db_overwrite_msgBox;
            //db_overwrite_msgBox.setGeometry(266,125,1142,633);
            db_overwrite_msgBox.setText("Overwrite Databases?");
            db_overwrite_msgBox.setInformativeText("All existing data will be lost, this cannot be undone");
            db_overwrite_msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            db_overwrite_msgBox.setDefaultButton(QMessageBox::No);
            db_overwrite_msgBox.setIcon(QMessageBox::Warning);
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
}

void SEGSAdminTool::create_databases(bool overwrite)
{
    ui->runDBTool->setEnabled(false);
    ui->runDBTool->setText("Please Wait...");
    ui->output->appendPlainText("Setting arguments...");
    qApp->processEvents();
    qDebug() << "Setting arguments...";
    QString program = "dbtool create";
    if (overwrite)
    {
        program.append(" -f");
    }
    #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    program.prepend("./");
    #endif
    m_createDB = new QProcess(this);
    m_createDB->start(program);

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
        emit checkForDB(true);
        ui->icon_status_db->setText("<html><head/><body><p><img src=':/icons/icon_good.png'/></p></body></html>");
        qApp->processEvents();
        emit addAdminUser();
    }
    else
    {
        ui->output->appendPlainText("Failed to start DBTool Add User...");
        qDebug() <<"Failed to start DBTool Add User...";
        emit checkForDB(true);
        qApp->processEvents();
    }
}

void SEGSAdminTool::read_createDB()
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

void SEGSAdminTool::is_server_running()
{
    if (m_server_running == true)
    {
        SEGSAdminTool::stop_auth_server();
    }
    else
    {
        SEGSAdminTool::start_auth_server();
    }
}
void SEGSAdminTool::start_auth_server()
{
    ui->output->appendPlainText("Setting arguments...");
    qApp->processEvents();
    QString program = "authserver";
    #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    program.prepend("./");
    #endif
    m_start_auth_server = new QProcess(this);
    m_start_auth_server->start(program);

    if (m_start_auth_server->waitForStarted())
    {

        ui->authserver_start->setText("Please Wait...");
        ui->output->appendPlainText("Starting AuthServer...");
        ui->authserver_status->setText("STARTING");
        ui->authserver_status->setStyleSheet("QLabel {color: rgb(255, 153, 51)}");
        qApp->processEvents();
        qDebug() << "Starting AuthServer...";
        connect(m_start_auth_server,&QProcess::readyReadStandardError,this,&SEGSAdminTool::read_authserver);
        connect(m_start_auth_server,&QProcess::readyReadStandardOutput,this,&SEGSAdminTool::read_authserver);
        m_start_auth_server->waitForFinished(2000);
        if(m_start_auth_server->state()==QProcess::Running)
        {
            ui->output->appendPlainText("*** AuthServer Running ***");
            ui->authserver_status->setText("RUNNING");
            ui->authserver_status->setStyleSheet("QLabel {color: rgb(0, 200, 0)}");
            ui->authserver_start->setStyleSheet("color: rgb(255, 0, 0);");
            ui->authserver_start->setText("Stop Server");
            ui->user_box->setEnabled(false);
            ui->server_setup_box->setEnabled(false);
            ui->server_config->setEnabled(false);
            int pid = m_start_auth_server->processId();
            qDebug()<<pid;
            m_server_running = true;
        }
        if(m_start_auth_server->state()==QProcess::NotRunning)
        {
            ui->output->appendPlainText("*** AUTHSERVER NOT RUNNING... Have you setup your piggs and settings files? ***");
            ui->authserver_start->setText("Start Server");
            ui->authserver_start->setEnabled(true);
            ui->authserver_status->setText("STOPPED");
            ui->authserver_status->setStyleSheet("QLabel {color: rgb(255, 0, 0)}");
            m_server_running = false;
        }
    }
    else
    {
        ui->output->appendPlainText("Failed to start AuthServer...");
        qDebug() <<"Failed to start AuthServer...";
        ui->authserver_start->setText("Start Server");
        ui->authserver_start->setEnabled(true);
        qApp->processEvents();
    }

}

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
        ui->authserver_status->setText("STOPPED");
        ui->authserver_status->setStyleSheet("QLabel {color: rgb(255, 0, 0)}");
        ui->authserver_start->setStyleSheet("color: rgb(0, 170, 0);");
        ui->authserver_start->setText("Start Server");
        ui->user_box->setEnabled(true);
        ui->server_setup_box->setEnabled(true);
        ui->server_config->setEnabled(true);
        m_server_running = false;
        ui->output->appendPlainText("*** AuthServer Stopped ***");
    }
}

void SEGSAdminTool::check_for_config_file() // Does this on application start
{
    // Load settings.cfg if exists
    ui->output->appendPlainText("Checking for existing configuration file...");
    QFileInfo config_file("settings.cfg");
    if (config_file.exists())
    {
        QString config_file_path = config_file.absoluteFilePath();
        ui->output->appendPlainText("SUCCESS: Configuration file found!");
        ui->icon_status_config->setText("<html><head/><body><p><img src=':/icons/icon_good.png'/></p></body></html>");
        ui->gen_config_file->setEnabled(false);
        ui->runDBTool->setEnabled(true);
        ui->set_up_data_button->setEnabled(true);
        ui->authserver_start->setEnabled(true);
        ui->settings_button->setEnabled(true);
        emit readyToRead(config_file_path);
    }
    else
    {
        ui->output->appendPlainText("WARNING: No settings.cfg file found! Please use the server setup to your left");
        ui->icon_status_config->setText("<html><head/><body><p><img src=':/icons/icon_warning.png'/></p></body></html>");
        ui->runDBTool->setEnabled(false); // Cannot create DB without settings.cfg
        ui->createUser->setEnabled(false); // Cannot create user without settings.cfg
        ui->set_up_data_button->setEnabled(false); // Shouldn't create data before config file exists
        ui->authserver_start->setEnabled(false); // Shouldn't run authserver if no config file exists
        ui->settings_button->setEnabled(false); // Shouldn't be able to edit settings if no config file exists
    }
}

//!@}

