/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>

#include <QComboBox>
#include <QProcess>
#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void selectionsFromCfg(QString sel);

protected:
    void changeEvent(QEvent *e);
    void updateState();

    bool selectAddressOrReportFailure(QComboBox *cmb, QString v, bool failedPreviously);
private slots:
    void on_loadConfigFile_clicked();

    void on_saveConfigFile_clicked();

    void on_startServerBtn_clicked();

    void on_stopServerBtn_clicked();

    void onServerDone(int retcode);
    void onServerStarted();
    void on_selectServerExePath_clicked();

    void onServerError(QProcess::ProcessError);
    void readyReadStandardOutput();
    void readyReadStandardError();
private:
    Ui::MainWindow *ui;
    QProcess *m_server_process;
    QString m_selected_config_file;
    QSettings *m_loaded_server_settings;
    QString m_last_config_path;
    QString m_server_executable;
};

#endif // MAINWINDOW_H
