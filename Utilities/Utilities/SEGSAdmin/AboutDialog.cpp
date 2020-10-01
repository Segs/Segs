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

#include "AboutDialog.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    QFont dejavu_font;
    dejavu_font.setFamily("DejaVu Sans Condensed");
    dejavu_font.setPointSize(12);
    ui->segs_copy->setFont(dejavu_font);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::show_ui()
{
    show();
}

//!@}
