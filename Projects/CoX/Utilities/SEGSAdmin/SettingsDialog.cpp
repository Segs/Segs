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
#include "Globals.h"
#include <QSettings>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>
#include <QValidator>
#include <QCheckBox>
#include <QMap>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->acc_dbdriver->addItems(g_db_drivers);
    ui->char_dbdriver->addItems(g_db_drivers);
    // Field Validators
    ui->map_player_fade_in_value->setValidator(new QIntValidator(0, 1000, this)); // Only allow int between 0 and 1000
    m_get_ip = new GetIPDialog(this);

    // SettingsDialog Signals
    connect(ui->ip_auto_populate,&QPushButton::clicked,m_get_ip,&GetIPDialog::get_local_ip);
    connect(ui->settings_save_button,&QPushButton::clicked,this,&SettingsDialog::field_validator);
    connect(ui->reset_defaults,&QPushButton::clicked,this,&SettingsDialog::set_default_values);
    connect(ui->map_player_fade_in,&QSlider::valueChanged,this,&SettingsDialog::text_edit_updater);
    connect(ui->map_player_fade_in_value,&QLineEdit::textChanged,this,&SettingsDialog::slider_updater);
    connect(ui->auto_logout_check,&QCheckBox::clicked,this,&SettingsDialog::auto_logout_checkbox_validator);

    // GetIP Signals
    connect(m_get_ip,&GetIPDialog::sendIP,this,&SettingsDialog::auto_populate_ip_main);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::text_edit_updater()
{
    QString fade_in_textedit = QString::number(ui->map_player_fade_in->value());
    ui->map_player_fade_in_value->setText(fade_in_textedit);
}

void SettingsDialog::slider_updater()
{
    int fade_in_slider = ui->map_player_fade_in_value->text().toInt();
    ui->map_player_fade_in->setValue(fade_in_slider);
}

void SettingsDialog::auto_logout_checkbox_validator()
{
    if(ui->auto_logout_check->isChecked())
    {
        ui->time_to_auto_logout_label->setEnabled(true);
        ui->time_to_auto_logout_spin->setEnabled(true);
    }
    else
    {
        ui->time_to_auto_logout_label->setEnabled(false);
        ui->time_to_auto_logout_spin->setEnabled(false);
    }
}

void SettingsDialog::open_settings_dialog()
{
    QFileInfo config_file("settings.cfg");
    QString config_file_path = config_file.absoluteFilePath();
    SettingsDialog::read_config_file(config_file_path);
    QList<QLineEdit *> all_line_edits = ui->tabWidget->findChildren<QLineEdit *>();
    foreach(QLineEdit* le, all_line_edits)
    {
        le->setStyleSheet("background-color: rgb(255, 255, 255)");
    }
    QString fade_in_value = QString::number(ui->map_player_fade_in->value());
    ui->map_player_fade_in_value->setText(fade_in_value);
    show();
    auto_logout_checkbox_validator();
}

void SettingsDialog::read_config_file(QString filePath)
{
    QSettings config_file(filePath, QSettings::IniFormat);
    config_file.beginGroup("AdminServer");
    config_file.beginGroup("AccountDatabase");
        QString acc_db_driver = config_file.value("db_driver","").toString();
        QString acc_db_host = config_file.value("db_host","").toString();
        int acc_db_port = config_file.value("db_port","").toInt();
        int acc_index = ui->acc_dbdriver->findText(acc_db_driver);
        ui->acc_dbdriver->setCurrentIndex(acc_index);
        ui->acc_dbhost->setText(acc_db_host);
        ui->acc_dbport->setValue(acc_db_port);
        config_file.endGroup();
        config_file.beginGroup("CharacterDatabase");
        QString char_db_driver = config_file.value("db_driver","").toString();
        QString char_db_host = config_file.value("db_host","").toString();
        int char_db_port = config_file.value("db_port","").toInt();
        int char_index = ui->char_dbdriver->findText(char_db_driver);
        ui->char_dbdriver->setCurrentIndex(char_index);
        ui->char_dbhost->setText(char_db_host);
        ui->char_dbport->setValue(char_db_port);
    config_file.endGroup(); // AdminServer
    config_file.endGroup(); // AccountDatabase
    config_file.beginGroup("AuthServer");
        QString auth_loc_addr = config_file.value("location_addr","").toString();
        QStringList auth_portip = auth_loc_addr.split(':');
        int auth_port = auth_portip[1].toInt();
        ui->auth_ip->setText(auth_portip[0]);
        ui->auth_port->setValue(auth_port);
    config_file.endGroup(); // AuthServer
    config_file.beginGroup("GameServer");
        QString game_listen_addr = config_file.value("listen_addr","").toString();
        QStringList game_listen_addr_portip = game_listen_addr.split(':');
        QString game_loc_addr = config_file.value("location_addr","").toString();
        QStringList game_loc_addr_portip = game_loc_addr.split(':');
        int game_loc_addr_port = game_loc_addr_portip[1].toInt();
        int game_listen_addr_port = game_listen_addr_portip[1].toInt();
        int max_players = config_file.value("max_players","").toInt();
        int max_char_slots = config_file.value("max_character_slots","").toInt();
        ui->game_listen_ip->setText(game_listen_addr_portip[0]);
        ui->game_listen_port->setValue(game_listen_addr_port);
        ui->game_loc_ip->setText(game_loc_addr_portip[0]);
        ui->game_loc_port->setValue(game_loc_addr_port);
        ui->game_max_players->setValue(max_players);
        ui->game_max_slots->setValue(max_char_slots);
    config_file.endGroup(); // GameServer
    config_file.beginGroup("MapServer");
        QString map_listen_addr = config_file.value("listen_addr","").toString();
        QStringList map_listen_addr_portip = map_listen_addr.split(':');
        int map_listen_addr_port = map_listen_addr_portip[1].toInt();
        QString map_loc_addr = config_file.value("location_addr","").toString();
        QStringList map_loc_addr_portip = map_loc_addr.split(':');
        int map_loc_addr_port = map_loc_addr_portip[1].toInt();
        QString maps_loc = config_file.value("maps","DefaultMapInstances").toString();
        float player_fade_in = config_file.value("player_fade_in", "").toFloat();
        float motd_timer = config_file.value("motd_timer", "").toFloat();
        ui->map_listen_ip->setText(map_listen_addr_portip[0]);
        ui->map_listen_port->setValue(map_listen_addr_port);
        ui->map_location_ip->setText(map_loc_addr_portip[0]);
        ui->map_location_port->setValue(map_loc_addr_port);
        ui->map_location->setText(maps_loc);
        ui->map_player_fade_in->setValue(player_fade_in);
        ui->map_motd_timer->setValue(motd_timer);
        ui->costume_slot_unlocks_edit->setText(QString(config_file.value("costume_slot_unlocks", "").toString()));
    config_file.endGroup(); // MapServer
    config_file.beginGroup("AFKSettings");
        ui->time_to_afk_spin->setValue(config_file.value("time_to_afk", "").toInt());
        ui->time_to_logout_msg_spin->setValue(config_file.value("time_to_logout_msg", "").toInt());
        ui->time_to_auto_logout_spin->setValue(config_file.value("time_to_auto_logout", "").toInt());
        ui->auto_logout_check->setChecked(config_file.value("uses_auto_logout", "").toBool());
    config_file.endGroup(); // AFKSettings
    config_file.beginGroup("StartingCharacter");
        ui->inherent_powers_edit->setText(config_file.value("inherent_powers", "").toString());
        ui->starting_temp_edit->setText(config_file.value("starting_temps", "").toString());
        ui->starting_insp_edit->setText(config_file.value("starting_inspirations", "").toString());
        ui->starting_level_spin->setValue(config_file.value("starting_level", "").toInt());
        ui->starting_inf_spin->setValue(config_file.value("starting_inf", "").toInt());
    config_file.endGroup(); // StartingCharacter
    config_file.beginGroup("Logging");
        // Read settings.cfg and populate UI elements
        QGridLayout *logging_grid = new QGridLayout(ui->logging_cats);
        logging_grid->setColumnStretch(0,1);
        logging_grid->setColumnStretch(1,1);
        logging_grid->setColumnStretch(2,1);
        for(int i = 0; i < config_file.childKeys().size(); ++i)
        {
            QString temp_key = config_file.childKeys().at(i);
            bool temp_value = config_file.value(temp_key).toBool();
            QCheckBox *logging_checkbox = new QCheckBox(temp_key);
            logging_grid->addWidget(logging_checkbox);
            logging_checkbox->setChecked(temp_value);
        }
    config_file.endGroup(); // Logging
}

void SettingsDialog::generate_default_config_file(QString ip)
{
    QSettings config_file_write("settings.cfg", QSettings::IniFormat);
    QSettings settings_template("settings_template.cfg", QSettings::IniFormat);
    config_file_write.beginGroup("AdminServer");
    config_file_write.beginGroup("AccountDatabase");
        config_file_write.setValue("db_driver","QSQLITE");
        config_file_write.setValue("db_host","127.0.0.1");
        config_file_write.setValue("db_port","5432");
        config_file_write.setValue("db_name","segs");
        config_file_write.setValue("db_user","segsadmin");
        config_file_write.setValue("db_pass","segs123");
    config_file_write.endGroup(); // AccountDatabase
    config_file_write.beginGroup("CharacterDatabase");
        config_file_write.setValue("db_driver","QSQLITE");
        config_file_write.setValue("db_host","127.0.0.1");
        config_file_write.setValue("db_port","5432");
        config_file_write.setValue("db_name","segs_game");
        config_file_write.setValue("db_user","segsadmin");
        config_file_write.setValue("db_pass","segs123");
    config_file_write.endGroup(); // CharacterDatabase
    config_file_write.endGroup(); // AdminServer
    config_file_write.beginGroup("AuthServer");
        config_file_write.setValue("location_addr",ip+":2106");
    config_file_write.endGroup(); // AuthServer
    config_file_write.beginGroup("GameServer");
        config_file_write.setValue("listen_addr",ip+":7002");
        config_file_write.setValue("location_addr",ip+":7002");
        config_file_write.setValue("max_players","200");
        config_file_write.setValue("max_character_slots","8");
    config_file_write.endGroup(); // GameServer
    config_file_write.beginGroup("MapServer");
        config_file_write.setValue("listen_addr",ip+":7003");
        config_file_write.setValue("location_addr",ip+":7003");
        config_file_write.setValue("maps","DefaultMapInstances");
        config_file_write.setValue("player_fade_in", "380.0");
        config_file_write.setValue("costume_slot_unlocks", "19,29,39,49");
    config_file_write.endGroup(); // MapServer
    config_file_write.beginGroup("AFKSettings");
        config_file_write.setValue("time_to_afk", "300");
        config_file_write.setValue("time_to_logout_msg", "1080");
        config_file_write.setValue("time_to_auto_logout", "120");
        config_file_write.setValue("uses_auto_logout", "true");
    config_file_write.endGroup(); // AFKSettings
    config_file_write.beginGroup("StartingCharacter");
        config_file_write.setValue("inherent_powers", "prestige_generic_Sprintp");
        config_file_write.setValue("starting_temps", "EMP_Glove,Cryoprojection_Bracers");
        config_file_write.setValue("starting_inspirations", "Resurgence,Phenomenal_Luck");
        config_file_write.setValue("starting_level", "1");
        config_file_write.setValue("starting_inf", "0");
    config_file_write.endGroup(); // StartingCharacter
    config_file_write.beginGroup("Logging");
        settings_template.beginGroup("Logging");
            QStringList logging_keys = settings_template.childKeys();
        settings_template.endGroup(); // settings_template Logging
        for (const QString &key : logging_keys)
        {
            config_file_write.setValue(key, false);
        }
    config_file_write.endGroup(); // Logging
    config_file_write.sync();
    emit checkForConfigFile();
    emit check_data_and_dir(ui->map_location->text());
}

void SettingsDialog::save_changes_config_file()
{
    QSettings config_file_write("settings.cfg", QSettings::IniFormat);
    config_file_write.beginGroup("AdminServer");
        config_file_write.beginGroup("AccountDatabase");
        config_file_write.setValue("db_driver",ui->acc_dbdriver->currentText());
        config_file_write.setValue("db_host",ui->acc_dbhost->text());
        config_file_write.setValue("db_port",ui->acc_dbport->text());
    config_file_write.endGroup(); // AccountDatabase
    config_file_write.beginGroup("CharacterDatabase");
        config_file_write.setValue("db_driver",ui->char_dbdriver->currentText());
        config_file_write.setValue("db_host",ui->char_dbhost->text());
        config_file_write.setValue("db_port",ui->char_dbport->text());
    config_file_write.endGroup(); // CharacterDatabase
    config_file_write.endGroup(); // AdminServer
    config_file_write.beginGroup("AuthServer");
        config_file_write.setValue("location_addr",ui->auth_ip->text()+":"+ui->auth_port->text());
    config_file_write.endGroup(); // AuthServer
    config_file_write.beginGroup("GameServer");
        config_file_write.setValue("listen_addr",ui->game_listen_ip->text()+":"+ui->game_listen_port->text());
        config_file_write.setValue("location_addr",ui->game_loc_ip->text()+":"+ui->game_loc_port->text());
        config_file_write.setValue("max_players",ui->game_max_players->text());
        config_file_write.setValue("max_character_slots",ui->game_max_slots->text());
        config_file_write.setValue("costume_slot_unlocks", ui->costume_slot_unlocks_edit->text());
    config_file_write.endGroup(); // GameServer
    config_file_write.beginGroup("MapServer");
        config_file_write.setValue("listen_addr",ui->map_listen_ip->text()+":"+ui->map_listen_port->text());
        config_file_write.setValue("location_addr",ui->map_location_ip->text()+":"+ui->map_location_port->text());
        config_file_write.setValue("maps",ui->map_location->text());
        QString player_fade_in = ui->map_player_fade_in_value->text() + ".0";
        config_file_write.setValue("player_fade_in",player_fade_in);
        QString motd_timer = ui->map_motd_timer->text() + ".0";
        config_file_write.setValue("motd_timer",motd_timer);
    config_file_write.endGroup(); // MapServer
    config_file_write.beginGroup("AFKSettings");
        config_file_write.setValue("time_to_afk", ui->time_to_afk_spin->value());
        config_file_write.setValue("time_to_logout_msg", ui->time_to_logout_msg_spin->value());
        config_file_write.setValue("time_to_auto_logout", ui->time_to_auto_logout_spin->value());
        config_file_write.setValue("uses_auto_logout", ui->auto_logout_check->isChecked());
    config_file_write.endGroup(); // AFKSettings
    config_file_write.beginGroup("StartingCharacter");
        config_file_write.setValue("inherent_powers", ui->inherent_powers_edit->text());
        config_file_write.setValue("starting_temps", ui->starting_temp_edit->text());
        config_file_write.setValue("starting_inspirations", ui->starting_insp_edit->text());
        config_file_write.setValue("starting_level", ui->starting_level_spin->value());
        config_file_write.setValue("starting_inf", ui->starting_inf_spin->value());
    config_file_write.endGroup(); // StartingCharacter
    config_file_write.beginGroup("Logging");
        QList<QCheckBox*> check_boxes = ui->tab_logging->findChildren<QCheckBox *>();
        for(int i = 0; i < check_boxes.size(); ++i)
        {
            if(check_boxes.at(i)->isChecked())
                config_file_write.setValue(check_boxes.at(i)->text(), true);
            else
                config_file_write.setValue(check_boxes.at(i)->text(), false);
        }
    config_file_write.endGroup(); // Logging
    config_file_write.sync();

    QMessageBox settings_saved;
    settings_saved.setText("Settings Saved");
    settings_saved.setStandardButtons(QMessageBox::Ok);
    settings_saved.setDefaultButton(QMessageBox::Ok);
    settings_saved.setIcon(QMessageBox::Information);
    settings_saved.exec();
    emit check_data_and_dir(ui->map_location->text());
}

void SettingsDialog::set_default_values()
{
    ui->acc_dbdriver->setCurrentText("QSQLITE");
    ui->acc_dbhost->setText("127.0.0.1");
    ui->acc_dbport->setValue(5432);
    ui->char_dbdriver->setCurrentText("QSQLITE");
    ui->char_dbhost->setText("127.0.0.1");
    ui->char_dbport->setValue(5432);
    ui->auth_ip->setText("127.0.0.1");
    ui->auth_port->setValue(2106);
    ui->game_listen_ip->setText("127.0.0.1");
    ui->game_listen_port->setValue(7002);
    ui->game_loc_ip->setText("127.0.0.1");
    ui->game_loc_port->setValue(7002);
    ui->game_max_players->setValue(200);
    ui->game_max_slots->setValue(8);
    ui->costume_slot_unlocks_edit->setText("19,29,39,49");
    ui->map_listen_ip->setText("127.0.0.1");
    ui->map_listen_port->setValue(7003);
    ui->map_location_ip->setText("127.0.0.1");
    ui->map_location_port->setValue(7003);
    ui->map_location->setText("DefaultMapInstances");
    ui->map_player_fade_in->setValue(380.0);
    ui->map_motd_timer->setValue(120.0);
    ui->time_to_afk_spin->setValue(300);
    ui->time_to_logout_msg_spin->setValue(1080);
    ui->time_to_auto_logout_spin->setValue(120);
    ui->auto_logout_check->setChecked(true);
    QList<QCheckBox*> check_boxes = ui->tab_logging->findChildren<QCheckBox *>();
    for(int i = 0; i < check_boxes.size(); ++i)
    {
        check_boxes.at(i)->setChecked(false);
    }
    ui->inherent_powers_edit->setText("prestige_generic_Sprintp");
    ui->starting_temp_edit->setText("EMP_Glove,Cryoprojection_Bracers");
    ui->starting_insp_edit->setText("Resurgence,Phenomenal_Luck");
    ui->starting_level_spin->setValue(1);
    ui->starting_inf_spin->setValue(0);
}

void SettingsDialog::field_validator()
{

    QList<QLineEdit *> all_line_edits = ui->tabWidget->findChildren<QLineEdit *>();
    bool all_fields_validated = true;
    foreach(QLineEdit* le, all_line_edits)
    {
        if(le->text().isEmpty()) // Checks for empty fields, if found highlight fields
        {
            le->setStyleSheet("background-color: rgb(252, 175, 62)");
            all_fields_validated = false;

        }
    }

    if(all_fields_validated == false) // Field validation failed, stop save
    {
        QMessageBox validation_error;
        validation_error.setText("The highlighted fields can not be blank");
        //validation_error.setText()
        validation_error.setStandardButtons(QMessageBox::Ok);
        validation_error.setDefaultButton(QMessageBox::Ok);
        validation_error.setIcon(QMessageBox::Warning);
        validation_error.exec();
    }
    else
    {
        SettingsDialog::save_changes_config_file(); // Field validation passed, proceed to save
    }
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
