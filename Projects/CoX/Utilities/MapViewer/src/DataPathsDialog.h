/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef DATAPATHSDIALOG_H
#define DATAPATHSDIALOG_H

#include <QDialog>

namespace Ui
{
    class DataPathsDialog;
}

class DataPathsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DataPathsDialog(QWidget *parent = nullptr);
    ~DataPathsDialog();

private slots:
    void on_selectUnpackedDir_clicked();

private:
    Ui::DataPathsDialog *ui;
};

#endif // DATAPATHSDIALOG_H
