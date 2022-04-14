/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
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
#include "NetworkManager.h"
#include "UpdateDetailDialog.h"
#include "AboutDialog.h"
#include "SelectScriptDialog.h"
#include "Helpers.h"
#include "Worker.h"

#include "Components/Settings.h"
#include "Version.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QtGlobal>
#include <QVersionNumber>


SEGSAdminTool::SEGSAdminTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SEGSAdminTool)
{
    ui->setupUi(this);
    ui->update_detail->setEnabled(false);
    QFont dejavu_font;
    dejavu_font.setFamily("DejaVu Sans Condensed");
    dejavu_font.setPointSize(12);
    ui->output->setFont(dejavu_font);
    ui->update_detail->setFont(dejavu_font);
    ui->output->appendPlainText("*** Welcome to SEGSAdmin ***");
    ui->segs_admin_version->setText(QString("v") + VersionInfo::getAuthVersionNumber());
    m_add_user_dialog = new AddNewUserDialog(this);
    m_set_up_data = new SetUpData(this);
    m_settings_dialog = new SettingsDialog(this);
    m_generate_config_dialog = new GenerateConfigFileDialog(this);
    m_network_manager = new NetworkManager();
    m_update_dialog = new UpdateDetailDialog(this);
    m_about_dialog = new AboutDialog(this);
    m_script_dialog = new SelectScriptDialog(this);

    // SEGSAdminTool Signals
    connect(this,&SEGSAdminTool::checkForDB,this,&SEGSAdminTool::check_db_exist);
    connect(this,&SEGSAdminTool::addAdminUser,m_add_user_dialog,&AddNewUserDialog::on_add_admin_user);
    connect(this,&SEGSAdminTool::checkForConfigFile,this,&SEGSAdminTool::check_for_config_file);
    connect(this,&SEGSAdminTool::getMapsDirConfigCheck,m_settings_dialog,&SettingsDialog::send_maps_dir_config_check); // May be a much better way to do this, but this works for now
    connect(this,&SEGSAdminTool::readyToRead,m_settings_dialog,&SettingsDialog::read_config_file);
    connect(this,&SEGSAdminTool::checkConfigVersion,this,&SEGSAdminTool::check_config_version);
    connect(this,&SEGSAdminTool::recreateConfig,m_generate_config_dialog,&GenerateConfigFileDialog::on_generate_config_file);
    connect(ui->actionAbout,&QAction::triggered,m_about_dialog,&AboutDialog::show_ui);
    connect(ui->update_detail,&QPushButton::clicked,m_update_dialog,&UpdateDetailDialog::show_update);
    connect(ui->createUser,&QPushButton::clicked,m_add_user_dialog,&AddNewUserDialog::on_add_user);
    connect(ui->runDBTool,&QPushButton::clicked,this,&SEGSAdminTool::check_db_exist);
    connect(ui->set_up_data_button,&QPushButton::clicked,m_set_up_data,&SetUpData::open_data_dialog);
    connect(ui->settings_button,&QPushButton::clicked,m_settings_dialog,&SettingsDialog::open_settings_dialog);
    connect(ui->gen_config_file,&QPushButton::clicked,m_generate_config_dialog,&GenerateConfigFileDialog::on_generate_config_file);
    connect(ui->authserver_start,&QPushButton::clicked,this,&SEGSAdminTool::is_server_running);
    connect(ui->motd_editor,&QPushButton::clicked,m_script_dialog,&SelectScriptDialog::show_dialog);

    // GenerateConfigFileDialog Signals
    connect(m_generate_config_dialog,&GenerateConfigFileDialog::sendInputConfigFile,m_settings_dialog,&SettingsDialog::generate_default_config_file);

    // AddNewUserDialog Signals
    connect(m_add_user_dialog,&AddNewUserDialog::sendInput,this,&SEGSAdminTool::commit_user);

    // SetUpData Signals
    connect(m_set_up_data,&SetUpData::dataSetupComplete,this,&SEGSAdminTool::checkDataAndDir);
    connect(m_set_up_data,&SetUpData::getMapsDir,m_settings_dialog,&SettingsDialog::send_maps_dir);

    // SettingsDialog Signals
    connect(m_settings_dialog,&SettingsDialog::checkForConfigFile,this,&SEGSAdminTool::check_for_config_file);
    connect(m_settings_dialog,&SettingsDialog::check_data_and_dir,this,&SEGSAdminTool::checkDataAndDir);
    connect(m_settings_dialog,&SettingsDialog::sendMapsDirConfigCheck,this,&SEGSAdminTool::checkDataAndDir);

    // Network Manager Signals
    connect(this,&SEGSAdminTool::getLatestReleases,m_network_manager,&NetworkManager::get_latest_releases);
    connect(m_network_manager,&NetworkManager::releasesReadyToRead,this,&SEGSAdminTool::readReleaseInfo);

    // Send startup signals
    emit checkForConfigFile();
    emit check_db_exist(true);
    emit getMapsDirConfigCheck();
    emit getLatestReleases();

}

SEGSAdminTool::~SEGSAdminTool()
{
    delete ui;
}

void SEGSAdminTool::checkDataAndDir() // Checks for data sub dirs and maps dir
{
    QPixmap check_icon(":icons/Resources/check.svg");
    QPixmap alert_triangle(":icons/Resources/alert-triangle.svg");
    ui->output->appendPlainText("Checking for correct data and maps directories...");
    QStringList data_dirs = {
        "data/bin",
        "data/converted",
        "data/ent_types",
        "data/geobin",
        "data/object_library",
        "data/scenes",
        "data/shaders",
        "data/texts"
    };
    bool all_maps_exist = true;
    for(const QString &map_name : g_map_names)
    {
        all_maps_exist &= QDir(Helpers::getMapsDir()+"/"+map_name).exists();
        if(!all_maps_exist)
            break; // at least one map dir is missing, we can finish early
    }
    bool all_data_dirs_exist = true;
    for(const QString &data_dir : data_dirs)
    {
        all_data_dirs_exist &= QDir(data_dir).exists();
        if(!all_data_dirs_exist)
        {
            ui->output->appendPlainText("Error: Data directory missing: " + data_dir);
            break; // at least one map dir is missing, we can finish early
        }
    }
    if(all_data_dirs_exist && all_maps_exist)
    {
        ui->icon_status_data->setPixmap(check_icon);
        ui->output->appendPlainText("SUCCESS: Data and maps directories found!");
    }
    else
    {
        ui->icon_status_data->setPixmap(alert_triangle);
        ui->output->appendPlainText("WARNING: We couldn't find the correct data and/or maps directories. Please use the server setup to your left");
    }
}

void SEGSAdminTool::commit_user(QString username, QString password, QString acclevel)
{
    ui->output->appendPlainText("Setting arguments...");
    ui->createUser->setEnabled(false);
    ui->createUser->setText("Please Wait...");
    qApp->processEvents();
    qDebug() << "Setting arguments...";

    QString program = "dbtool";
    QStringList arguments = {"adduser", "-l", username, "-p", password, "-a", acclevel};
    #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
        program.prepend("./");
    #endif
    m_createUser = new QProcess(this);
    m_createUser->start(program, arguments);

    if(m_createUser->waitForStarted())
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
    ui->output->insertPlainText(output_err);
    ui->output->insertPlainText(output_std);
    ui->output->moveCursor(QTextCursor::End);
}

void SEGSAdminTool::check_db_exist(bool on_startup)
{
    QPixmap check_icon(":icons/Resources/check.svg");
    QPixmap alert_triangle(":icons/Resources/alert-triangle.svg");
    ui->output->appendPlainText("Checking for existing databases...");
    qDebug() << "Checking for existing databases...";

    QSettings config(Settings::getSettingsPath(), QSettings::IniFormat, nullptr);

    QFileInfo file1(config.value(QStringLiteral("AdminServer/AccountDatabase/db_name"), "segs.db").toString());
    QFileInfo file2(config.value(QStringLiteral("AdminServer/CharacterDatabase/db_name"), "segs_game.db").toString());
    if(on_startup) // Runs this check on startup or for checking creation in other methods
    {
        if(file1.exists() && file2.exists())
        {
            ui->output->appendPlainText("SUCCESS: Existing databases found!");
            ui->icon_status_db->setPixmap(check_icon);
            ui->runDBTool->setText("Create New Databases");
            ui->runDBTool->setEnabled(true);
            ui->createUser->setEnabled(true);
        }
        else
        {
            ui->output->appendPlainText("WARNING: Not all databases were found. Please use the server setup to your left");
            ui->icon_status_db->setPixmap(alert_triangle);
            ui->createUser->setEnabled(false); // Cannot create users until DB's created
        }
    }
    else
    {
        if(file1.exists() || file2.exists())
        {
            QMessageBox db_overwrite_msgBox;
            //db_overwrite_msgBox.setGeometry(266,125,1142,633);
            db_overwrite_msgBox.setText("Overwrite Databases?");
            db_overwrite_msgBox.setInformativeText("All existing data will be lost, this cannot be undone");
            db_overwrite_msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            db_overwrite_msgBox.setDefaultButton(QMessageBox::No);
            db_overwrite_msgBox.setIcon(QMessageBox::Warning);
            int confirm = db_overwrite_msgBox.exec();
            switch (confirm)
            {
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
    QPixmap check_icon(":icons/Resources/check.svg");
    QPixmap alert_triangle(":icons/Resources/alert-triangle.svg");
    ui->runDBTool->setEnabled(false);
    ui->runDBTool->setText("Please Wait...");
    ui->output->appendPlainText("Setting arguments...");
    qApp->processEvents();
    qDebug() << "Setting arguments...";
    QString program = "dbtool";
    QStringList arguments = {"create"};
    if(overwrite)
    {
        arguments.append("-f");
    }
    #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    program.prepend("./");
    #endif
    m_createDB = new QProcess(this);
    m_createDB->start(program, arguments);

    if(m_createDB->waitForStarted())
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
        ui->icon_status_db->setPixmap(check_icon);
        qApp->processEvents();
        emit addAdminUser();
    }
    else
    {
        ui->output->appendPlainText("Failed to start DBTool Add User... "
                                    "**Please ensure you are NOT running as an administrator or sudo user**");
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
    ui->output->insertPlainText(output_err);
    ui->output->insertPlainText(output_std);
    ui->output->moveCursor(QTextCursor::End);
    qApp->processEvents();
}

void SEGSAdminTool::is_server_running()
{
    if(m_server_running == true)
    {
        SEGSAdminTool::stop_segs_server();
    }
    else
    {
        SEGSAdminTool::start_segs_server();
    }
}
void SEGSAdminTool::start_segs_server()
{
    ui->output->appendPlainText("Setting arguments...");
    qApp->processEvents();
    QString program = "segs_server";
    QStringList arguments = {""};
    #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    program.prepend("./");
    #endif
    m_start_segs_server = new QProcess(this);
    m_start_segs_server->start(program, arguments);

    if(m_start_segs_server->waitForStarted())
    {

        ui->authserver_start->setText("Please Wait...");
        ui->output->appendPlainText("Starting SEGS Server...");
        ui->authserver_status->setText("STARTING");
        ui->authserver_status->setStyleSheet("QLabel {color: rgb(255, 153, 51)}");
        qApp->processEvents();
        qDebug() << "Starting SEGS Server...";
        connect(m_start_segs_server,&QProcess::readyReadStandardError,this,&SEGSAdminTool::read_segsserver);
        connect(m_start_segs_server,&QProcess::readyReadStandardOutput,this,&SEGSAdminTool::read_segsserver);
        m_start_segs_server->waitForFinished(2000);
        if(m_start_segs_server->state()==QProcess::Running)
        {
            ui->output->appendPlainText("*** SEGS Server Running ***");
            ui->authserver_status->setText("RUNNING");
            ui->authserver_status->setStyleSheet("QLabel {color: rgb(0, 200, 0)}");
            ui->authserver_start->setStyleSheet("color: rgb(255, 0, 0);");
            ui->authserver_start->setText("Stop Server");
            ui->authserver_start->setIcon(QIcon(":/icons/Resources/square.svg"));
            ui->user_box->setEnabled(false);
            ui->server_setup_box->setEnabled(false);
            ui->server_config->setEnabled(false);
            m_server_running = true;
        }
        if(m_start_segs_server->state()==QProcess::NotRunning)
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
        ui->output->appendPlainText("Failed to start SEGS Server...");
        qDebug() <<"Failed to start SEGS Server...";
        ui->authserver_start->setText("Start Server");
        ui->authserver_start->setEnabled(true);
        qApp->processEvents();
    }

}

void SEGSAdminTool::read_segsserver()
{
    QByteArray out_err = m_start_segs_server->readAllStandardError();
    QByteArray out_std = m_start_segs_server->readAllStandardOutput();
    qDebug().noquote()<<QString(out_err);
    qDebug().noquote()<<QString(out_std);
    QString output_err = out_err;
    QString output_std = out_std;
    output_err.replace("Press ENTER to continue...", "** FINISHED **");
    output_std.replace("Press ENTER to continue...", "** FINISHED **");
    ui->output->insertPlainText(output_err);
    ui->output->insertPlainText(output_std);
    ui->output->moveCursor(QTextCursor::End);
    qApp->processEvents();
}

void SEGSAdminTool::stop_segs_server()
{
    m_start_segs_server->close();
    if(m_start_segs_server->state()==QProcess::Running)
    {
        ui->output->appendPlainText("*** SEGS Server Failed to Stop ***");
    }
    if(m_start_segs_server->state()==QProcess::NotRunning)
    {
        ui->authserver_start->setEnabled(true);
        ui->authserver_status->setText("STOPPED");
        ui->authserver_status->setStyleSheet("QLabel {color: rgb(255, 0, 0)}");
        ui->authserver_start->setStyleSheet("color: rgb(0, 170, 0);");
        ui->authserver_start->setText("Start Server");
        ui->authserver_start->setIcon(QIcon(":/icons/Resources/play.svg"));
        ui->user_box->setEnabled(true);
        ui->server_setup_box->setEnabled(true);
        ui->server_config->setEnabled(true);
        m_server_running = false;
        ui->output->appendPlainText("*** SEGS Server Stopped ***");
    }
}

void SEGSAdminTool::check_for_config_file() // Does this on application start
{
    QPixmap check_icon(":icons/Resources/check.svg");
    QPixmap alert_triangle(":icons/Resources/alert-triangle.svg");
    // Load settings.cfg if exists
    ui->output->appendPlainText("Checking for existing configuration file...");
    QFileInfo config_file(Settings::getSettingsPath());
    if(config_file.exists())
    {
        QString config_file_path = config_file.absoluteFilePath();
        ui->output->appendPlainText("SUCCESS: Configuration file found!");
        emit checkConfigVersion(config_file_path);
        ui->icon_status_config->setPixmap(check_icon);
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
        ui->icon_status_config->setPixmap(alert_triangle);
        ui->runDBTool->setEnabled(false); // Cannot create DB without settings.cfg
        ui->createUser->setEnabled(false); // Cannot create user without settings.cfg
        ui->set_up_data_button->setEnabled(false); // Shouldn't create data before config file exists
        ui->authserver_start->setEnabled(false); // Shouldn't run authserver if no config file exists
        ui->settings_button->setEnabled(false); // Shouldn't be able to edit settings if no config file exists
    }
}

void SEGSAdminTool::check_config_version(QString filePath)
{
    ui->output->appendPlainText("Checking configuration version...");

    QSettings config_file(filePath, QSettings::IniFormat);
    config_file.beginGroup("MetaData");
    int config_version = config_file.value("config_version","").toInt();
    config_file.endGroup();

    if (config_version != VersionInfo::getConfigVersion())
    {
        ui->output->appendPlainText("WARNING: Configuration file version incorrect or missing. Prompting for recreation");
        QMessageBox::StandardButton ask_recreate_config = QMessageBox::warning(this,
                    "Config File Version Incorrect", "Your settings.cfg may be out of date. Do you want to to recreate?"
                    "\n\nWARNING: All settings will be overwritten",
                    QMessageBox::Yes | QMessageBox::No);

        if(ask_recreate_config == QMessageBox::Yes)
        {
            ui->output->appendPlainText("Recreating settings.cfg");
            emit recreateConfig();
        }
        else
        {
            ui->output->appendPlainText("Not Recreating settings.cfg");
        }
    }
    else
    {
        ui->output->appendPlainText("SUCCESS: Configuration file version correct");
    }
}

void SEGSAdminTool::readReleaseInfo(const QString &error)
{
    ui->output->appendPlainText("Checking for Updates...");
    QString version_number = VersionInfo::getAuthVersionNumber();
    if(!g_segs_release_info.isEmpty())
    {
        QVersionNumber installed_version = QVersionNumber::fromString(version_number);
        QVersionNumber latest_version = QVersionNumber::fromString(g_segs_release_info[0].tag_name.remove(QChar('v')));
        int compare_version = QVersionNumber::compare(installed_version, latest_version);
        ui->update_detail->setText("Checking for updates...");

        if(compare_version < 0)
        {
            ui->output->appendPlainText("New Update Found!");
            ui->update_detail->setEnabled(true);
            ui->update_detail->setStyleSheet("color: rgb(204, 0, 0)");
            ui->update_detail->setText("Update available!");
            ui->update_detail->setFlat(false);
        }
        else
        {
            ui->output->appendPlainText("No updates available");
            ui->update_detail->setText("Up to date");
            ui->update_detail->setStyleSheet("color: rgb(0, 200, 0)");
        }
    }
    else
    {
        ui->update_detail->setText("Error fetching latest releases");
        ui->update_detail->setStyleSheet("color: rgb(255, 0, 0)");
        ui->output->appendPlainText("*********************************WARNING***********************************\n"
                                    "There was an error fetching the latest release information. You may be missing"
                                    " the OpenSSL library files. Please refer to README.MD for more info\n"
                                    "You can view the latest releases by visiting https://github.com/Segs/Segs/releases"
                                    "\n******************************************************************************"
                                    "\nError Message: " + error);

    }

}

//!@}
