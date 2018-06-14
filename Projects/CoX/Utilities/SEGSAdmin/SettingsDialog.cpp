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

#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "GetIPDialog.h"
#include <QSettings>
#include <QMessageBox>
#include <QFileInfo>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    m_get_ip = new GetIPDialog(this);

    // SettingsDialog Signals
    connect(ui->buttonBox,&QDialogButtonBox::accepted,this,&SettingsDialog::save_changes_config_file);
    connect(ui->ip_auto_populate,&QPushButton::clicked,m_get_ip,&GetIPDialog::get_local_ip);

    // GetIP Signals
    connect(m_get_ip,&GetIPDialog::sendIP,this,&SettingsDialog::auto_populate_ip_main);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::open_settings_dialog()
{
    QFileInfo config_file("settings.cfg");
    QString config_file_path = config_file.absoluteFilePath();
    SettingsDialog::read_config_file(config_file_path);
    show();
}

void SettingsDialog::read_config_file(QString filePath)
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
    QString maps_loc = config_file.value("maps","DefaultMapInstances").toString();
    QString player_fade_in = config_file.value("player_fade_in", "").toString();
    ui->map_listen_ip->setText(map_listen_addr_portip[0]);
    ui->map_listen_port->setText(map_listen_addr_portip[1]);
    ui->map_location_ip->setText(map_loc_addr_portip[0]);
    ui->map_location_port->setText(map_loc_addr_portip[1]);
    ui->map_location->setText(maps_loc);
    ui->map_player_fade_in->setText(player_fade_in);
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
    ui->log_slashcommand->setChecked(config_file.value("log_slashcommands","").toBool());
    ui->log_description->setChecked(config_file.value("log_description","").toBool());
    ui->log_friends->setChecked(config_file.value("log_friends","").toBool());
    ui->log_minimap->setChecked(config_file.value("log_minimap","").toBool());
    ui->log_lfg->setChecked(config_file.value("log_lfg","").toBool());
    ui->log_npcs->setChecked(config_file.value("log_npcs","").toBool());
    ui->log_animations->setChecked(config_file.value("log_animations","").toBool());
}

void SettingsDialog::generate_default_config_file(QString server_name, QString ip)
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
    config_file_write.setValue("maps","DefaultMapInstances");
    config_file_write.setValue("player_fade_in", "380.0");
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
    emit check_data_and_dir(ui->map_location->text());
}

void SettingsDialog::save_changes_config_file()
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
    config_file_write.setValue("player_fade_in",ui->map_player_fade_in->text());
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
    QMessageBox settings_saved;
    settings_saved.setText("Settings Saved");
    settings_saved.setStandardButtons(QMessageBox::Ok);
    settings_saved.setDefaultButton(QMessageBox::Ok);
    settings_saved.setIcon(QMessageBox::Information);
    settings_saved.exec();
    emit check_data_and_dir(ui->map_location->text());
}

void SettingsDialog::auto_populate_ip_main(QString local_ip)
{
    ui->game_listen_ip->setText(local_ip);
    ui->game_loc_ip->setText(local_ip);
    ui->map_listen_ip->setText(local_ip);
    ui->map_location_ip->setText(local_ip);
    ui->auth_ip->setText(local_ip);
}

void SettingsDialog::send_maps_dir()
{
    QString maps_dir = ui->map_location->text();
    emit sendMapsDir(maps_dir);
}

void SettingsDialog::send_maps_dir_config_check()
{
    QString maps_dir = ui->map_location->text();
    emit sendMapsDirConfigCheck(maps_dir);
}

//!@}
