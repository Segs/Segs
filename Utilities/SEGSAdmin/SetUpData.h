/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef SETUPDATA_H
#define SETUPDATA_H

#include <QDialog>
#include <QProcess>
#include <QFontDatabase>
#include <QThread>


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
    QThread worker_thread;

public slots:
    void selectPiggsDir();
    void open_data_dialog();
    bool copyPiggFiles();
    bool createDefaultDirectorys();
    bool checkClientVersion();
    void extractPiggs();
    void logUIMessage(QString message);
    void updateUIPercentage(int progress);
    void dataReady();

signals:
    bool callPiggWorker();
    void getMapsDir();
    void quitPiggLoop();
    void workerCompleted();
    void dataSetupComplete();

private:
    Ui::SetUpData *ui;
};

#endif // SETUPDATA_H
