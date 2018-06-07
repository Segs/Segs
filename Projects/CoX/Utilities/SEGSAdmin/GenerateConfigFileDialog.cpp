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

#include "GenerateConfigFileDialog.h"
#include "ui_GenerateConfigFileDialog.h"
#include "SEGSAdminTool.h"
#include "GetIPDialog.h"

GenerateConfigFileDialog::GenerateConfigFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GenerateConfigFileDialog)
{
    ui->setupUi(this);
    // Field Validators
    ui->config_server_name_edit->setMaxLength(32);
    connect(ui->buttonBox,&QDialogButtonBox::accepted,this,&GenerateConfigFileDialog::capture_input);
    // GetIP Signals/Slots
    m_get_ip = new GetIPDialog(this);
    connect(ui->ip_auto_populate,&QPushButton::clicked,m_get_ip,&GetIPDialog::get_local_ip);
    connect(m_get_ip,&GetIPDialog::sendIP,this,&GenerateConfigFileDialog::auto_populate_ip_gen_config);
}

GenerateConfigFileDialog::~GenerateConfigFileDialog()
{
    delete ui;
}

void GenerateConfigFileDialog::on_generate_config_file()
{
    ui->config_ip_edit->clear();
    ui->config_server_name_edit->clear();
    show();
}

void GenerateConfigFileDialog::capture_input()
{
    QString server_name = ui->config_server_name_edit->text();
    QString ip = ui->config_ip_edit->text();
    emit sendInputConfigFile(server_name,ip);
}

void GenerateConfigFileDialog::auto_populate_ip_gen_config(QString local_ip)
{
    ui->config_ip_edit->setText(local_ip);
}

//!@}
