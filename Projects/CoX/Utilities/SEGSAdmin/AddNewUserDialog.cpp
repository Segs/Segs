/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup SEGSAdmin Projects/CoX/Utilities/SEGSAdmin
 * @{
 */

#include "AddNewUserDialog.h"
#include "ui_AddNewUserDialog.h"
#include "SEGSAdminTool.h"
#include <QDebug>
#include <QMessageBox>

AddNewUserDialog::AddNewUserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddNewUserDialog)
{
    ui->setupUi(this);
    // Field Validators
    ui->usernameedit->setMaxLength(14);
    ui->passedit->setMaxLength(14);
    connect(ui->buttonBox,&QDialogButtonBox::accepted,this,&AddNewUserDialog::capture_input);

}
AddNewUserDialog::~AddNewUserDialog()
{
    delete ui;
}

void AddNewUserDialog::on_add_user()
{
    ui->label->clear();
    ui->usernameedit->clear();
    ui->passedit->clear();
    ui->accleveledit->setValue(1);
    ui->accleveledit->show();
    ui->acclevel->show();

    show();
}

void AddNewUserDialog::on_add_admin_user()
{
    ui->label->setText("Create an admin user");
    ui->usernameedit->clear();
    ui->passedit->clear();
    ui->accleveledit->setValue(9);
    ui->accleveledit->hide();
    ui->acclevel->hide();
    show();
}

void AddNewUserDialog::capture_input()
{
    QString username = ui->usernameedit->text();
    QString password = ui->passedit->text();
    QString acclevel = ui->accleveledit->text();
    if(username.toLatin1() == username && password.toLatin1() == password)
    {
        emit sendInput(username,password,acclevel);
    }
    else
    {
        QMessageBox invalid_chars_msgBox;
        invalid_chars_msgBox.setText("Invalid Characters");
        invalid_chars_msgBox.setInformativeText("Username and Password must contain valid characters");
        invalid_chars_msgBox.setStandardButtons(QMessageBox::Ok);
        invalid_chars_msgBox.setDefaultButton(QMessageBox::Ok);
        invalid_chars_msgBox.setIcon(QMessageBox::Warning);
        invalid_chars_msgBox.exec();
    }
}

//!@}
