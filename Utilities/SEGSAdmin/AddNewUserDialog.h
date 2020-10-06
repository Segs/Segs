/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef AddNewUserDialog_H
#define AddNewUserDialog_H

#include <QDialog>
#include <QProcess>

namespace Ui {
class AddNewUserDialog;
}

class AddNewUserDialog : public QDialog
{
    Q_OBJECT


public:
    explicit AddNewUserDialog(QWidget *parent = nullptr);
    ~AddNewUserDialog();
    void capture_input();

public Q_SLOTS:
    void on_add_user();
    void on_add_admin_user();

private:
    Ui::AddNewUserDialog *ui;

signals:
    void sendInput(QString username, QString password, QString acclevel);

};

#endif // AddNewUserDialog_H
