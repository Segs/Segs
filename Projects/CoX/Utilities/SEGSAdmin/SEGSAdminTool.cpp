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
#include "GenerateConfigFileDialog.h"
#include "SetUpData.h"
#include "Globals.h"
#include <QDebug>
#include <QtGlobal>
#include <QProcess>
#include <QFileInfo>
#include <QFile>
#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <QDir>
#include <iostream>

SEGSAdminTool::SEGSAdminTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SEGSAdminTool)

{
    ui->setupUi(this);
    ui->output->appendPlainText("*** Welcome to SEGSAdmin ***");
    ui->authserver_stop->setEnabled(false);
    // AddNewUserDialog Signals/Slots
    m_add_user_dialog = new AddNewUserDialog;
    connect(ui->createUser,&QPushButton::clicked,m_add_user_dialog,&AddNewUserDialog::on_add_user);
    connect(this,&SEGSAdminTool::addAdminUser,m_add_user_dialog,&AddNewUserDialog::on_add_admin_user);
    connect(m_add_user_dialog,&AddNewUserDialog::sendInput,this,&SEGSAdminTool::commit_user);
    // GenerateConfigFileDialog Signals/Slots
    m_generate_config_dialog = new GenerateConfigFileDialog;
    connect(m_generate_config_dialog,&GenerateConfigFileDialog::sendInputConfigFile,this,&SEGSAdminTool::generate_default_config_file);
    connect(ui->gen_config_file,&QPushButton::clicked,m_generate_config_dialog,&GenerateConfigFileDialog::on_generate_config_file);    
    // Local Signals/Slots
    connect(ui->runDBTool,&QPushButton::clicked,this,&SEGSAdminTool::check_db_exist);
    connect(this,&SEGSAdminTool::checkForDB,this,&SEGSAdminTool::check_db_exist);
    connect(ui->authserver_start,&QPushButton::clicked,this,&SEGSAdminTool::start_auth_server);
    connect(ui->authserver_stop,&QPushButton::clicked,this,&SEGSAdminTool::stop_auth_server);
    connect(this,&SEGSAdminTool::readyToRead,this,&SEGSAdminTool::read_config_file);
    connect(this,&SEGSAdminTool::checkForConfigFile,this,&SEGSAdminTool::check_for_config_file);
    connect(ui->settings_buttonBox,&QDialogButtonBox::accepted,this,&SEGSAdminTool::save_changes_config_file);
    connect(ui->logging_buttonBox,&QDialogButtonBox::accepted,this,&SEGSAdminTool::save_changes_config_file);
    // SetUpData Signals/Slots
    m_set_up_data = new SetUpData;
    connect(ui->set_up_data_button,&QPushButton::clicked,m_set_up_data,&SetUpData::open_data_dialog);
    connect(m_set_up_data,&SetUpData::dataSetupComplete,this,&SEGSAdminTool::check_data_and_dir);
    connect(m_set_up_data,&SetUpData::getMapsDir,this,&SEGSAdminTool::send_maps_dir);
    connect(this,&SEGSAdminTool::sendMapsDir,m_set_up_data,&SetUpData::create_default_directory);
    // Send startup signals
    emit checkForConfigFile();
    emit check_db_exist(true);
    emit check_data_and_dir();
}

SEGSAdminTool::~SEGSAdminTool()
{
    delete ui;
}

void SEGSAdminTool::check_data_and_dir() // Checks for data sub dirs and maps dir
{
    QStringList data_dirs = {"data/bin","data/geobin","data/object_library"}; // Currently only checking for these 3 sub dirs, check for all files could be overkill
    bool all_maps_exist = true;
    for(const QString &map_name : g_map_names)
    {
        all_maps_exist &= QDir(ui->map_location->text()+"/"+map_name).exists();
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
    }
    else
    {
        ui->icon_status_data->setText("<html><head/><body><p><img src=':/icons/icon_warning.png'/></p></body></html>");
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
            ui->output->appendPlainText("** Databases found **");
            ui->icon_status_db->setText("<html><head/><body><p><img src=':/icons/icon_good.png'/></p></body></html>");
            ui->runDBTool->setText("Create New Databases");
            ui->createUser->setEnabled(true);
        }
        else
        {
            ui->output->appendPlainText("** Not all databases found **");
            ui->icon_status_db->setText("<html><head/><body><p><img src=':/icons/icon_warning.png'/></p></body></html>");
            ui->createUser->setEnabled(false); // Cannot create users until DB's created
        }
    }
    else
    {
        if (file1.exists() || file2.exists())
        {
            QMessageBox db_overwrite_msgBox;
            db_overwrite_msgBox.setText("Overwrite Databases?");
            db_overwrite_msgBox.setInformativeText("All existing data will be lost, this cannot be undone");
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

void SEGSAdminTool::start_auth_server()
{
    ui->authserver_start->setEnabled(false);
    ui->authserver_start->setText("Please Wait...");
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
        ui->output->appendPlainText("Starting AuthServer...");
        ui->authserver_status->setText("STARTING");
        ui->authserver_status->setStyleSheet("QLabel {color: rgb(255, 153, 51)}");
        qApp->processEvents();
        qDebug() << "Starting AuthServer...";
        connect(m_start_auth_server,&QProcess::readyReadStandardError,this,&SEGSAdminTool::read_authserver);
        connect(m_start_auth_server,&QProcess::readyReadStandardOutput,this,&SEGSAdminTool::read_authserver);
        ui->authserver_start->setText("Start Server");
        ui->authserver_stop->setEnabled(true);
        qApp->processEvents();
        m_start_auth_server->waitForFinished(2000);
        if(m_start_auth_server->state()==QProcess::Running)
        {
            ui->output->appendPlainText("*** AuthServer Running ***");
            ui->authserver_status->setText("RUNNING");
            ui->authserver_status->setStyleSheet("QLabel {color: rgb(0, 200, 0)}");
            ui->tabWidget->setEnabled(false);
            ui->user_box->setEnabled(false);
            ui->server_setup_box->setEnabled(false);
            int pid = m_start_auth_server->processId();
            qDebug()<<pid;
        }
        if(m_start_auth_server->state()==QProcess::NotRunning)
        {
            ui->output->appendPlainText("*** AUTHSERVER NOT RUNNING... Have you setup your piggs and settings files? ***");
            ui->authserver_start->setText("Start Server");
            ui->authserver_start->setEnabled(true);
            ui->authserver_status->setText("STOPPED");
            ui->authserver_status->setStyleSheet("QLabel {color: rgb(255, 0, 0)}");
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
        ui->authserver_stop->setEnabled(false);
        ui->authserver_status->setText("STOPPED");
        ui->authserver_status->setStyleSheet("QLabel {color: rgb(255, 0, 0)}");
        ui->tabWidget->setEnabled(true);
        ui->user_box->setEnabled(true);
        ui->server_setup_box->setEnabled(true);
        ui->output->appendPlainText("*** AuthServer Stopped ***");
    }
}

void SEGSAdminTool::check_for_config_file() // Does this on application start
{
    // Load settings.cfg if exists
    QFileInfo config_file("settings.cfg");
    if (config_file.exists())
    {
        QString config_file_path = config_file.absoluteFilePath();
        ui->icon_status_config->setText("<html><head/><body><p><img src=':/icons/icon_good.png'/></p></body></html>");
        ui->gen_config_file->setEnabled(false);
        ui->runDBTool->setEnabled(true);
        ui->set_up_data_button->setEnabled(true);
        ui->tab_settings->setEnabled(true);
        ui->tab_logging->setEnabled(true);
        ui->authserver_start->setEnabled(true);
        emit readyToRead(config_file_path);
    }
    else
    {
        ui->output->appendPlainText("** No settings.cfg file found! **");
        ui->icon_status_config->setText("<html><head/><body><p><img src=':/icons/icon_warning.png'/></p></body></html>");
        ui->runDBTool->setEnabled(false); // Cannot create DB without settings.cfg
        ui->createUser->setEnabled(false); // Cannot create user without settings.cfg
        ui->set_up_data_button->setEnabled(false); // Shouldn't create data before config file exists
        ui->authserver_start->setEnabled(false); // Shouldn't run authserver if no config file exists
        ui->tab_settings->setEnabled(false);
        ui->tab_logging->setEnabled(false);

    }
}

void SEGSAdminTool::read_config_file(QString filePath)
{
    QSettings config_file(filePath, QSettings::IniFormat);
    config_file.beginGroup("AdminServer");
    config_file.beginGroup("AccountDatabase");
    QString acc_db_driver = config_file.value("db_driver","").toString();
    QString acc_db_host = config_file.value("db_host","").toString();
    QString acc_db_port = config_file.value("db_port","").toString();
    ui->acc_dbdriver->setText(acc_db_driver);
    ui->acc_dbhost->setText(acc_db_host);
    ui->acc_dbport->setText(acc_db_port);
    config_file.endGroup();
    config_file.beginGroup("CharacterDatabase");
    QString char_db_driver = config_file.value("db_driver","").toString();
    QString char_db_host = config_file.value("db_host","").toString();
    QString char_db_port = config_file.value("db_port","").toString();
    ui->char_dbdriver->setText(char_db_driver);
    ui->char_dbhost->setText(char_db_host);
    ui->char_dbport->setText(char_db_port);
    config_file.endGroup();
    config_file.endGroup();
    config_file.beginGroup("AuthServer");
    QString auth_loc_addr = config_file.value("location_addr","").toString();
    QStringList auth_portip = auth_loc_addr.split(':');
    ui->auth_ip->setText(auth_portip[0]);
    ui->auth_port->setText(auth_portip[1]);
    config_file.endGroup();
    config_file.beginGroup("GameServer");
    QString game_server_name = config_file.value("server_name","").toString();
    QString game_listen_addr = config_file.value("listen_addr","").toString();
    QStringList game_listen_addr_portip = game_listen_addr.split(':');
    QString game_loc_addr = config_file.value("location_addr","").toString();
    QStringList game_loc_addr_portip = game_loc_addr.split(':');
    QString max_players = config_file.value("max_players","").toString();
    QString max_char_slots = config_file.value("max_character_slots","").toString();
    ui->game_server_name->setText(game_server_name);
    ui->game_listen_ip->setText(game_listen_addr_portip[0]);
    ui->game_listen_port->setText(game_listen_addr_portip[1]);
    ui->game_loc_ip->setText(game_loc_addr_portip[0]);
    ui->game_loc_port->setText(game_loc_addr_portip[1]);
    ui->game_max_players->setText(max_players);
    ui->game_max_slots->setText(max_char_slots);
    config_file.endGroup();
    config_file.beginGroup("MapServer");
    QString map_listen_addr = config_file.value("listen_addr","").toString();
    QStringList map_listen_addr_portip = map_listen_addr.split(':');
    QString map_loc_addr = config_file.value("location_addr","").toString();
    QStringList map_loc_addr_portip = map_loc_addr.split(':');
    QString maps_loc = config_file.value("maps","").toString();
    ui->map_listen_ip->setText(map_listen_addr_portip[0]);
    ui->map_listen_port->setText(map_listen_addr_portip[1]);
    ui->map_location_ip->setText(map_loc_addr_portip[0]);
    ui->map_location_port->setText(map_loc_addr_portip[1]);
    ui->map_location->setText(maps_loc);
    config_file.endGroup();
    config_file.beginGroup("Logging");
    ui->log_logging->setChecked(config_file.value("log_logging","").toBool());
    ui->log_keybinds->setChecked(config_file.value("log_keybinds","").toBool());
    ui->log_settings->setChecked(config_file.value("log_settings","").toBool());
    ui->log_gui->setChecked(config_file.value("log_gui","").toBool());
    ui->log_teams->setChecked(config_file.value("log_teams","").toBool());
    ui->log_db->setChecked(config_file.value("log_db","").toBool());
    ui->log_input->setChecked(config_file.value("log_input","").toBool());
    ui->log_position->setChecked(config_file.value("log_position","").toBool());
    ui->log_orientation->setChecked(config_file.value("log_orientation","").toBool());
    ui->log_chat->setChecked(config_file.value("log_chat","").toBool());
    ui->log_infomsg->setChecked(config_file.value("log_infomsg","").toBool());
    ui->log_emotes->setChecked(config_file.value("log_emotes","").toBool());
    ui->log_target->setChecked(config_file.value("log_target","").toBool());
    ui->log_spawn->setChecked(config_file.value("log_spawn","").toBool());
    ui->log_mapevents->setChecked(config_file.value("log_mapevents","").toBool());
    ui->log_slashcommand->setChecked(config_file.value("log_slashcommand","").toBool());
    ui->log_description->setChecked(config_file.value("log_description","").toBool());
    ui->log_friends->setChecked(config_file.value("log_friends","").toBool());
    ui->log_minimap->setChecked(config_file.value("log_minimap","").toBool());
    ui->log_lfg->setChecked(config_file.value("log_lfg","").toBool());
    ui->log_npcs->setChecked(config_file.value("log_npcs","").toBool());
    ui->log_animations->setChecked(config_file.value("log_animations","").toBool());
}

void SEGSAdminTool::generate_default_config_file(QString server_name, QString ip)
{
    QSettings config_file_write("settings.cfg", QSettings::IniFormat);
    config_file_write.beginGroup("AdminServer");
    config_file_write.beginGroup("AccountDatabase");
    config_file_write.setValue("db_driver","QSQLITE");
    config_file_write.setValue("db_host","127.0.0.1");
    config_file_write.setValue("db_port","5432");
    config_file_write.setValue("db_name","segs");
    config_file_write.setValue("db_user","segsadmin");
    config_file_write.setValue("db_pass","segs123");
    config_file_write.endGroup();
    config_file_write.beginGroup("CharacterDatabase");
    config_file_write.setValue("db_driver","QSQLITE");
    config_file_write.setValue("db_host","127.0.0.1");
    config_file_write.setValue("db_port","5432");
    config_file_write.setValue("db_name","segs_game");
    config_file_write.setValue("db_user","segsadmin");
    config_file_write.setValue("db_pass","segs123");
    config_file_write.endGroup();
    config_file_write.endGroup();
    config_file_write.beginGroup("AuthServer");
    config_file_write.setValue("location_addr",ip+":2106");
    config_file_write.endGroup();
    config_file_write.beginGroup("GameServer");
    config_file_write.setValue("server_name",server_name);
    config_file_write.setValue("listen_addr",ip+":7002");
    config_file_write.setValue("location_addr",ip+":7002");
    config_file_write.setValue("max_players","200");
    config_file_write.setValue("max_character_slots","8");
    config_file_write.endGroup();
    config_file_write.beginGroup("MapServer");
    config_file_write.setValue("listen_addr",ip+":7003");
    config_file_write.setValue("location_addr",ip+":7003");
    config_file_write.setValue("maps","maps");
    config_file_write.endGroup();
    config_file_write.beginGroup("Logging");
    config_file_write.setValue("log_generic","*.debug=true\nqt.*.debug=false");
    config_file_write.setValue("log_logging","false");
    config_file_write.setValue("log_keybinds","false");
    config_file_write.setValue("log_settings","false");
    config_file_write.setValue("log_gui","false");
    config_file_write.setValue("log_teams","false");
    config_file_write.setValue("log_db","false");
    config_file_write.setValue("log_input","false");
    config_file_write.setValue("log_position","false");
    config_file_write.setValue("log_orientation","false");
    config_file_write.setValue("log_chat","false");
    config_file_write.setValue("log_infomsg","false");
    config_file_write.setValue("log_emotes","false");
    config_file_write.setValue("log_target","false");
    config_file_write.setValue("log_spawn","false");
    config_file_write.setValue("log_mapevents","false");
    config_file_write.setValue("log_slashcommands","false");
    config_file_write.setValue("log_description","false");
    config_file_write.setValue("log_friends","false");
    config_file_write.setValue("log_minimap","false");
    config_file_write.setValue("log_lfg","false");
    config_file_write.setValue("log_npcs","false");
    config_file_write.setValue("log_animations","false");
    config_file_write.endGroup();

    config_file_write.sync();
    emit checkForConfigFile();
}

void SEGSAdminTool::save_changes_config_file()
{
    QSettings config_file_write("settings.cfg", QSettings::IniFormat);
    config_file_write.beginGroup("AdminServer");
    config_file_write.beginGroup("AccountDatabase");
    config_file_write.setValue("db_driver",ui->acc_dbdriver->text());
    config_file_write.setValue("db_host",ui->acc_dbhost->text());
    config_file_write.setValue("db_port",ui->acc_dbport->text());
    config_file_write.endGroup();
    config_file_write.beginGroup("CharacterDatabase");
    config_file_write.setValue("db_driver",ui->char_dbdriver->text());
    config_file_write.setValue("db_host",ui->char_dbhost->text());
    config_file_write.setValue("db_port",ui->char_dbport->text());
    config_file_write.endGroup();
    config_file_write.endGroup();
    config_file_write.beginGroup("AuthServer");
    config_file_write.setValue("location_addr",ui->auth_ip->text()+":"+ui->auth_port->text());
    config_file_write.endGroup();
    config_file_write.beginGroup("GameServer");
    config_file_write.setValue("server_name",ui->game_server_name->text());
    config_file_write.setValue("listen_addr",ui->game_listen_ip->text()+":"+ui->game_listen_port->text());
    config_file_write.setValue("location_addr",ui->game_loc_ip->text()+":"+ui->game_loc_port->text());
    config_file_write.setValue("max_players",ui->game_max_players->text());
    config_file_write.setValue("max_character_slots",ui->game_max_slots->text());
    config_file_write.endGroup();
    config_file_write.beginGroup("MapServer");
    config_file_write.setValue("listen_addr",ui->map_listen_ip->text()+":"+ui->map_listen_port->text());
    config_file_write.setValue("location_addr",ui->map_location_ip->text()+":"+ui->map_location_port->text());
    config_file_write.setValue("maps",ui->map_location->text());
    config_file_write.endGroup();
    config_file_write.beginGroup("Logging");
    config_file_write.setValue("log_generic","*.debug=true\nqt.*.debug=false");
    config_file_write.setValue("log_logging",ui->log_logging->isChecked());
    config_file_write.setValue("log_keybinds",ui->log_keybinds->isChecked());
    config_file_write.setValue("log_settings",ui->log_settings->isChecked());
    config_file_write.setValue("log_gui",ui->log_gui->isChecked());
    config_file_write.setValue("log_teams",ui->log_teams->isChecked());
    config_file_write.setValue("log_db",ui->log_db->isChecked());
    config_file_write.setValue("log_input",ui->log_input->isChecked());
    config_file_write.setValue("log_position",ui->log_position->isChecked());
    config_file_write.setValue("log_orientation",ui->log_orientation->isChecked());
    config_file_write.setValue("log_chat",ui->log_chat->isChecked());
    config_file_write.setValue("log_infomsg",ui->log_infomsg->isChecked());
    config_file_write.setValue("log_emotes",ui->log_emotes->isChecked());
    config_file_write.setValue("log_target",ui->log_target->isChecked());
    config_file_write.setValue("log_spawn",ui->log_spawn->isChecked());
    config_file_write.setValue("log_mapevents",ui->log_mapevents->isChecked());
    config_file_write.setValue("log_slashcommands",ui->log_slashcommand->isChecked());
    config_file_write.setValue("log_description",ui->log_description->isChecked());
    config_file_write.setValue("log_friends",ui->log_friends->isChecked());
    config_file_write.setValue("log_minimap",ui->log_minimap->isChecked());
    config_file_write.setValue("log_lfg",ui->log_lfg->isChecked());
    config_file_write.setValue("log_npcs",ui->log_npcs->isChecked());
    config_file_write.setValue("log_animations",ui->log_animations->isChecked());
    config_file_write.endGroup();

    config_file_write.sync();
    ui->output->appendPlainText("** Settings Saved **");
    emit checkForConfigFile();
    emit check_data_and_dir();
}

void SEGSAdminTool::send_maps_dir()
{
    QString maps_dir = ui->map_location->text();
    emit sendMapsDir(maps_dir);
}

//!@}

