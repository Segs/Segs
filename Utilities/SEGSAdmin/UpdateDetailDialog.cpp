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

#include "UpdateDetailDialog.h"
#include "ui_UpdateDetailDialog.h"
#include "Globals.h"
#include <QUrl>
#include <QDesktopServices>

UpdateDetailDialog::UpdateDetailDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDetailDialog)
{
    ui->setupUi(this);
    QFont dejavu_font;
    dejavu_font.setFamily("DejaVu Sans Condensed");
    dejavu_font.setPointSize(12);
    ui->update_notes->setFont(dejavu_font);
    connect(ui->download_button,&QPushButton::clicked,this,&UpdateDetailDialog::open_download_url);

}

UpdateDetailDialog::~UpdateDetailDialog()
{
    delete ui;
}

void UpdateDetailDialog::show_update()
{
    ui->version_label->setText(g_segs_release_info[0].tag_name);
    ui->version_name->setText(g_segs_release_info[0].release_name);
    ui->update_notes->setText(g_segs_release_info[0].release_notes);
    show();
}

void UpdateDetailDialog::open_download_url()
{
    QDesktopServices::openUrl((QUrl(g_segs_release_info[0].github_url)));
}

//!@}
