/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef SETUPDATA_H
#define SETUPDATA_H

#include <QDialog>
#include <QProcess>
#include <QFontDatabase>


namespace Ui {
class SetUpData;
}

class SetUpData : public QDialog
{
    Q_OBJECT
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

public:
    explicit SetUpData(QWidget *parent = 0);
    ~SetUpData();

public slots:
    void select_piggs_dir();
    void open_data_dialog();
    void copy_piggs_files();
    void pigg_dispatcher();
    void pigg_tool_worker(QString program);
    void create_default_directory(QString maps_dir);
    void read_piggtool();
    void pigg_dispatcher_wait_dialog();
    void check_client_version();

signals:
    void fileCopyComplete();
    void callPiggWorker(QString program);
    void dataSetupComplete(QString maps_dir);
    void getMapsDir();
    void quitPiggLoop();
    void readyToCopy();

private:
    Ui::SetUpData *ui;
    QProcess *m_pigg_tool;

};

#endif // SETUPDATA_H
