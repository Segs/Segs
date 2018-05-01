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

GenerateConfigFileDialog::GenerateConfigFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GenerateConfigFileDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox,&QDialogButtonBox::accepted,this,&GenerateConfigFileDialog::capture_input);
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
//!@}
