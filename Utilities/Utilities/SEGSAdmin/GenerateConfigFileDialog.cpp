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
    show();
}

void GenerateConfigFileDialog::capture_input()
{
    QString ip = ui->config_ip_edit->text();
    emit sendInputConfigFile(ip);
}

void GenerateConfigFileDialog::auto_populate_ip_gen_config(QString local_ip)
{
    ui->config_ip_edit->setText(local_ip);
}

//!@}
