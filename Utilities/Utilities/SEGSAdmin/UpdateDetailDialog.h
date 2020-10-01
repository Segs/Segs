/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef UPDATEDETAILDIALOG_H
#define UPDATEDETAILDIALOG_H

#include <QDialog>

namespace Ui {
class UpdateDetailDialog;
}

class UpdateDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDetailDialog(QWidget *parent = 0);
    ~UpdateDetailDialog();

public slots:
    void show_update();
    void open_download_url();

private:
    Ui::UpdateDetailDialog *ui;
};

#endif // UPDATEDETAILDIALOG_H
