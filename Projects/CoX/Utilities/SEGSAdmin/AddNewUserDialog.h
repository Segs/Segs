/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
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
