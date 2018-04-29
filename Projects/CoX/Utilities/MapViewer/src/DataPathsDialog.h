/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
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
