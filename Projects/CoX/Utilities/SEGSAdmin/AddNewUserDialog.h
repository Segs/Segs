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
    explicit AddNewUserDialog(QWidget *parent = 0);
    ~AddNewUserDialog();
    void capture_input();

public Q_SLOTS:
    void on_add_user();

private:
    Ui::AddNewUserDialog *ui;

signals:
    void sendInput(const QString username, const QString password, const QString acclevel);

};

#endif // AddNewUserDialog_H
