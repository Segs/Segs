/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup ServerMonitor Utilities/ServerMonitor
 * @{
 */

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QtNetwork/QNetworkInterface>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QTemporaryFile>
#include <cassert>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    bool has_at_least_one_non_loop_address = false;
    QList<QNetworkInterface> ifaces=QNetworkInterface::allInterfaces();
    for(int i=0; i<ifaces.size(); ++i) {
        QNetworkInterface ifc = ifaces.at(i);

        if(ifc.flags() & QNetworkInterface::IsUp) {
            QList<QNetworkAddressEntry> address_entries = ifc.addressEntries();
            for(int j=0; j<address_entries.size(); ++j) {
                if(address_entries.at(j).ip().protocol()& QAbstractSocket::IPv6Protocol)
                    continue;
                QString ipaddr = address_entries.at(j).ip().toString();
                if(ipaddr!="127.0.0.1") {
                    has_at_least_one_non_loop_address = true;
                    ui->authServerAddrSelector->addItem(ipaddr);
                    ui->mapServerAddrSelector->addItem(ipaddr);
                    ui->gameServerAddrSelector->addItem(ipaddr);

                }
            }
        }
    }
    if(!has_at_least_one_non_loop_address) {
        QMessageBox::critical(NULL,tr("Critical Error"),tr("No non-loopback adresses are available, we cannot continue."));
        QCoreApplication::exit(-1);
    }


    QSettings set;
    m_last_config_path = set.value("LastConfigPath").toString();
    m_server_executable = set.value("ServerExecutable","SELECT EXECUTABLE").toString();
    if(m_server_executable!="SELECT EXECUTABLE") {
        QDir cur=QDir::current();
        ui->serverExePath->setText(cur.relativeFilePath(m_server_executable));
    }
    else
        ui->serverExePath->setText(m_server_executable);

    if(m_last_config_path.isEmpty())
        m_last_config_path = QDir::currentPath();

    QString last_config = set.value("LastConfigFile","").toString();
    if( !last_config.isEmpty() ) {
        selectionsFromCfg(last_config);
        m_selected_config_file = last_config;

    }
    else
        m_loaded_server_settings = NULL;

    m_server_process = NULL;
    updateState();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_loaded_server_settings;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::updateState() {
    ui->startServerBtn->setEnabled(m_server_process==NULL && m_server_executable!="SELECT EXECUTABLE");
    ui->stopServerBtn->setEnabled(m_server_process!=NULL);
    ui->selectServerExePath->setEnabled(m_server_process==NULL);
    ui->saveConfigFile->setEnabled((!m_server_process && !m_selected_config_file.isEmpty()));
    ui->loadConfigFile->setEnabled(!m_server_process);
}
static QString baseAdressOrFailure(QString v) {
    QStringList parts = v.split(':');
    if(parts.size()!=2) {
        qWarning() << "Bad setting for address "<< v <<" found in config file";
        return "";
    }
    return parts.first();
}

bool MainWindow::selectAddressOrReportFailure(QComboBox *cmb,QString v, bool failedPreviously) {
    for(int i=0; i<cmb->count(); ++i) {
        if(cmb->itemText(i)==v) {
            cmb->setCurrentIndex(i);
            return false;
        }
    }
    if(!failedPreviously) {
        QMessageBox::warning(NULL,tr("Config file trouble"),QString(tr("Cannot find previously configured address %1")).arg(v));
    }
    if(cmb->count())
        cmb->setCurrentIndex(0);
    return true;
}
void MainWindow::selectionsFromCfg(QString sel)
{
    m_loaded_server_settings = new QSettings(sel,QSettings::IniFormat);
    m_loaded_server_settings->beginGroup("AuthServer");
    QString auth_srv = m_loaded_server_settings->value("listen_addr").toString();
    m_loaded_server_settings->endGroup();
    m_loaded_server_settings->beginGroup("GameServer");
    QString game_srv = m_loaded_server_settings->value("listen_addr").toString();
    m_loaded_server_settings->endGroup();
    m_loaded_server_settings->beginGroup("MapServer");
    QString map_srv = m_loaded_server_settings->value("listen_addr").toString();
    m_loaded_server_settings->endGroup();
    bool hadErrors = false;
    hadErrors = selectAddressOrReportFailure(ui->authServerAddrSelector,baseAdressOrFailure(auth_srv),hadErrors);
    hadErrors = selectAddressOrReportFailure(ui->gameServerAddrSelector,baseAdressOrFailure(game_srv),hadErrors);
    hadErrors = selectAddressOrReportFailure(ui->mapServerAddrSelector,baseAdressOrFailure(map_srv),hadErrors);

    QDir cur=QDir::current();
    ui->selectedConfigFile->setText(cur.relativeFilePath(sel));
}

void MainWindow::on_loadConfigFile_clicked()
{
    QString sel = QFileDialog::getOpenFileName(this,tr("Select config file"),m_last_config_path,
                                               tr("Server config files (*.cfg)"));
    QFileInfo fi(sel);
    m_last_config_path = fi.absolutePath();
    if( !sel.isEmpty() && fi.exists())
    {
        m_selected_config_file = sel;
        delete m_loaded_server_settings;
        selectionsFromCfg(sel);


        QSettings set;
        set.setValue("LastConfigPath",m_last_config_path);
        set.setValue("LastConfigFile",sel);
    }
    updateState();
}

void MainWindow::on_saveConfigFile_clicked()
{
    //TODO: stupid ini processing - differences between qt and ace..

    if(!m_loaded_server_settings) {
        QMessageBox::warning(NULL,tr("Config file trouble"),tr("No file was loaded, try loading one before editing and saving"));
        return;
    }
    QString auth_srv = ui->authServerAddrSelector->currentText()+":2006";
    QString game_srv = ui->gameServerAddrSelector->currentText()+":7002";
    QString map_srv = ui->mapServerAddrSelector->currentText()+":7003";
    QFile srcini(m_selected_config_file);
    if(!srcini.open(QFile::ReadOnly)) {
        QMessageBox::warning(NULL,tr("Config file trouble"),tr("Cannot open cfg file while saving changes"));
        return;
    }
    QTemporaryFile destini;
    if(!destini.open()) {
        QMessageBox::warning(NULL,tr("Config file trouble"),tr("Cannot create temp cfg file while saving changes"));
        return;
    }
    QString data = srcini.readAll();
    srcini.close();

    QStringList lines = data.split("\n");
    int ctx=0;
    for(int line_idx=0; line_idx<lines.size(); ++line_idx) {
        QString line = lines.at(line_idx);
        if(line.startsWith("[AuthServer]")) {
            ctx = 1;
        } else if(line.startsWith("[GameServer]")) {
            ctx = 2;
        } else if(line.startsWith("[MapServer]")) {
            ctx = 3;
        }
        if(ctx) {
            QString sel_addr;
            switch(ctx) {
                case 1:  sel_addr=  auth_srv; break;
                case 2:  sel_addr=  game_srv; break;
                case 3:  sel_addr=  map_srv; break;
            }
            if(line.startsWith("listen_addr")) {
                line = QString("listen_addr = \"%1\"").arg(sel_addr);
            } else if(line.startsWith("location_addr")) {
                line = QString("location_addr = \"%1\"").arg(sel_addr);
            }
        }
        destini.write(line.toLocal8Bit()+"\n");
    }
    delete m_loaded_server_settings; // ensure m_selected_config_file is closed at this point
    {
        QFile dest(m_selected_config_file);
        if(!dest.remove()) {
            QMessageBox::warning(NULL,tr("Config file trouble"),tr("Cannot remove old cfg file\n")+dest.errorString());
        }
    }
    destini.flush();
    if(!destini.rename(m_selected_config_file)) { // move created ini over the previous version
        QMessageBox::warning(NULL,tr("Config file trouble"),tr("Cannot move temp cfg file over the old one: file open somewhere else ?\n")+destini.errorString());
        qDebug() << destini.errorString();
    }
    destini.setAutoRemove(false); // was renamed to actual file do not remove it when destroying the object
    m_loaded_server_settings = new QSettings(m_selected_config_file,QSettings::IniFormat);


}
void MainWindow::onServerStarted() {
    updateState();
}
void MainWindow::onServerDone(int retcode) {
    delete m_server_process;
    m_server_process = NULL;
    qDebug() << "Server finished with " << retcode;
    updateState();
}
void MainWindow::onServerError(QProcess::ProcessError e) {
    ui->serverOutput->insertPlainText("ERR>!");
}
void 	MainWindow::readyReadStandardOutput()
{
  qDebug() << "readyOut";
  QProcess *p = (QProcess *)sender();
  QByteArray buf = p->readAllStandardOutput();
  ui->serverOutput->insertPlainText("1>"+buf);
  qDebug() << buf;
}
void 	MainWindow::readyReadStandardError()
{
  qDebug() << "readyErr";
  QProcess *p = (QProcess *)sender();
  QByteArray buf = p->readAllStandardError();
  ui->serverOutput->insertPlainText("2>"+buf);

  qDebug() << buf;
}

void MainWindow::on_startServerBtn_clicked()
{
    QString progname = ui->serverExePath->text();
    if(progname.isEmpty() || progname=="SELECT EXECUTABLE") {
        return ;
    }
    QStringList args;
    args.push_back("-f");
    args.push_back(m_selected_config_file);

    m_server_process = new QProcess(this);
    connect(m_server_process,SIGNAL(started()),SLOT(onServerStarted()));
    connect(m_server_process,SIGNAL(finished(int)),SLOT(onServerDone(int)));
    connect(m_server_process,SIGNAL(error(QProcess::ProcessError)),SLOT(onServerError(QProcess::ProcessError)));
    connect(m_server_process,SIGNAL(readyReadStandardOutput()),SLOT(readyReadStandardOutput()));
    connect(m_server_process,SIGNAL(readyReadStandardError()),SLOT(readyReadStandardError()));
    m_server_process->setWorkingDirectory(QFileInfo(progname).absolutePath());
    m_server_process->setProgram(m_server_executable);
    m_server_process->setEnvironment(QProcess::systemEnvironment());
    m_server_process->setArguments(args);
    m_server_process->start();
}

void MainWindow::on_stopServerBtn_clicked()
{
    if(m_server_process) {
        disconnect(m_server_process,SIGNAL(started()),this,SLOT(onServerStarted()));
        disconnect(m_server_process,SIGNAL(finished(int)),this,SLOT(onServerDone(int)));
        disconnect(m_server_process,SIGNAL(error(QProcess::ProcessError)),this,SLOT(onServerError(QProcess::ProcessError)));
        disconnect(m_server_process,SIGNAL(readyReadStandardOutput()),this,SLOT(readyReadStandardOutput()));
        disconnect(m_server_process,SIGNAL(readyReadStandardError()),this,SLOT(readyReadStandardError()));
        onServerDone(0);
    }
    if(m_server_process && m_server_process->isOpen()) {
        m_server_process->terminate();
    }
    delete m_server_process;
}

void MainWindow::on_selectServerExePath_clicked()
{
    QFileDialog dlg(NULL,tr("Select server executable"),m_last_config_path);
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setFilter(QDir::Executable);
    if(QDialog::Accepted!=dlg.exec()) {
        return;
    }
    QString sel = dlg.selectedFiles().first();
    QFileInfo fi(sel);
    m_last_config_path = fi.absolutePath();


    if(!sel.isEmpty()) {
        m_server_executable = sel;
        QDir cur=QDir::current();
        fi.filePath();
        ui->serverExePath->setText(cur.relativeFilePath(sel));
        QSettings set;
        set.setValue("ServerExecutable",sel);
    }
    updateState();
}

//! @}
