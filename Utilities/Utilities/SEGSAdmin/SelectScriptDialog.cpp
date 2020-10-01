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

#include "SelectScriptDialog.h"
#include "ui_SelectScriptDialog.h"
#include "TextEdit.h"
#include <QDebug>
#include <QMessageBox>

SelectScriptDialog::SelectScriptDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectScriptDialog)
{
    ui->setupUi(this);
    m_text_editor = new TextEdit();
    connect(ui->load_button,&QPushButton::clicked,this,&SelectScriptDialog::open_editor);
    connect(this,&SelectScriptDialog::loadScript,m_text_editor,&TextEdit::show_text_editor);
}

SelectScriptDialog::~SelectScriptDialog()
{
    delete ui;
}

void SelectScriptDialog::show_dialog()
{
    show();
    populate_dropdown();
}

void SelectScriptDialog::populate_dropdown()
{
    ui->script_select->clear();

    // Available scripts can be managed here
    m_segs_scripts = new(QMap<QString, QString>);
    m_segs_scripts->insert("-- Select --", "");
    m_segs_scripts->insert("Message of the day (MOTD)","scripts/motd.smlx");
    m_segs_scripts->insert("Tutorial", "scripts/tutorial.smlx");

    QStringList m_segs_scripts_keys = m_segs_scripts->keys();
    ui->script_select->addItems(m_segs_scripts_keys);

}

void SelectScriptDialog::open_editor()
{
    if(ui->script_select->currentText() == "-- Select --")
    {
        QMessageBox select_script_msgbox;
        select_script_msgbox.setText("Please select a script from the available options");
        select_script_msgbox.setStandardButtons(QMessageBox::Ok);
        select_script_msgbox.setDefaultButton(QMessageBox::Ok);
        select_script_msgbox.setIcon(QMessageBox::Warning);
        select_script_msgbox.setModal(true);
        select_script_msgbox.exec();
    }
    else
    {
        QString selected_script = m_segs_scripts->value(ui->script_select->currentText());
        emit loadScript(selected_script);
        close();
    }
}

//!@}
