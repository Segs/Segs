/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef SELECTSCRIPTDIALOG_H
#define SELECTSCRIPTDIALOG_H

#include <QDialog>

namespace Ui {
class SelectScriptDialog;
}

class SelectScriptDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectScriptDialog(QWidget *parent = nullptr);
    ~SelectScriptDialog();
    class TextEdit *m_text_editor;


public slots:
    void show_dialog();
    void open_editor();

signals:
    void loadScript(const QString &script);

private:
    Ui::SelectScriptDialog *ui;
    QMap<QString, QString> *m_segs_scripts;
    void populate_dropdown();
};

#endif // SELECTSCRIPTDIALOG_H
