/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapViewer Projects/CoX/Utilities/MapViewer
 * @{
 */

#include "DataPathsDialog.h"
#include "ui_DataPathsDialog.h"

#include <QFileDialog>
#include <QSettings>

extern QString basepath;

DataPathsDialog::DataPathsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DataPathsDialog)
{
    ui->setupUi(this);
    QSettings our_settings(QSettings::IniFormat,QSettings::UserScope,"SEGS","MapViewer");
    QString data_dir=our_settings.value("ExtractedDir",QString()).toString();
    ui->unpackPathTxt->setText(data_dir);
}

DataPathsDialog::~DataPathsDialog()
{
    delete ui;
}

void DataPathsDialog::on_selectUnpackedDir_clicked()
{
    QString dir= QFileDialog::getExistingDirectory();
    if(dir.isEmpty())
        return;
    ui->unpackPathTxt->setText(dir);
    QSettings our_settings(QSettings::IniFormat,QSettings::UserScope,"SEGS","MapViewer");
    our_settings.setValue("ExtractedDir",dir);
    basepath = dir;
}

//! @}
