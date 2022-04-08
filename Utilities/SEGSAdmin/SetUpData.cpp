/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup SEGSAdmin Projects/CoX/Utilities/SEGSAdmin
 * @{
 */

#include "SetUpData.h"
#include "ui_SetUpData.h"
#include "SEGSAdminTool.h"
#include "Globals.h"
#include "Worker.h"
#include "Helpers.h"

#include "Components/Settings.h"


#include <QFileDialog>
#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <QApplication>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QSettings>

SetUpData::SetUpData(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetUpData)
{
    ui->setupUi(this);
    QFont dejavu_font;
    dejavu_font.setFamily("DejaVu Sans Condensed");
    dejavu_font.setPointSize(12);
    ui->piggtool_output->setFont(dejavu_font);
    connect(ui->pigg_select_file,&QToolButton::clicked,this,&SetUpData::selectPiggsDir);
    connect(ui->copy_extract_button,&QPushButton::clicked,this,&SetUpData::extractPiggs);
}

SetUpData::~SetUpData()
{
    worker_thread.quit();  // Gracefully stop thread when component destroyed
    worker_thread.wait();
    delete ui;
}

void SetUpData::open_data_dialog()
{
    ui->pigg_file_url->clear();
    ui->copy_extract_button->setEnabled(false);
    ui->progressBar->setValue(0);
    ui->progressBar_piggtool->setValue(0);
    ui->progressBar->setEnabled(false);
    ui->label_extract_pigg->setEnabled(false);
    ui->progressBar_piggtool->setEnabled(false);
    ui->label_create_directory->setEnabled(false);
    ui->icon_cox_directory->hide();
    ui->icon_extract_pigg->hide();
    ui->icon_create_directory->hide();
    show();
}

void SetUpData::logUIMessage(QString message)
{
    ui->piggtool_output->appendPlainText(message);
}

void SetUpData::updateUIPercentage(int progress)
{
    ui->progressBar_piggtool->setValue(progress);
}

void SetUpData::selectPiggsDir()
{
    QString home_dir = QStandardPaths::locate(QStandardPaths::HomeLocation, QString(),
                                              QStandardPaths::LocateDirectory);
    QString pigg_dir = QFileDialog::getExistingDirectory(this, tr("Select CoX Directory"),
                                                         home_dir,
                                                         QFileDialog::ShowDirsOnly);
    ui->pigg_file_url->setText(pigg_dir);
    ui->copy_extract_button->setEnabled(true);
}

void SetUpData::extractPiggs()
{
    // Check client version
    if(!checkClientVersion())
        logUIMessage("Error when checking client version");

    // Copy piggs to working directory
    if(!copyPiggFiles())
        logUIMessage("Error copying pigg files");

    // Create default maps directory

    if(!createDefaultDirectorys())
        logUIMessage("Error creating default directories");

    // Extract piggs

    ui->label_extract_pigg->setEnabled(true);
    ui->progressBar_piggtool->setEnabled(true);

    // Prepare seperate thread and signals/slots for pigg file(s) extraction
    Worker *worker = new Worker;
    worker->moveToThread(&worker_thread); // Sending this to another thread here
    connect(&worker_thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &SetUpData::callPiggWorker, worker, &Worker::piggDispatcher);
    connect(worker, &Worker::sendUIMessage, this, &SetUpData::logUIMessage);
    connect(worker, &Worker::sendUIPercentage, this, &SetUpData::updateUIPercentage);
    connect(worker, &Worker::dataReady, this, &SetUpData::dataReady);
    worker_thread.start();

    emit callPiggWorker();
}

bool SetUpData::checkClientVersion() // Generate SHA-1 hash of CoX.exe and compare against hash set below, checks if client version is correct.
{
    QString cox_exe = ui->pigg_file_url->text().append("/CoX.exe");
    QCryptographicHash hash(QCryptographicHash::Sha1);
    QFile file(cox_exe);

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"Cannot find or open CoX.exe";
        ui->piggtool_output->appendPlainText("Cannot find or open CoX.exe, did you pick the correct directory?");
        return false;
    }
    qDebug()<<"Generating SHA-1 Hash of CoX.exe";
    ui->piggtool_output->appendPlainText("Checking CoX client is correct version");
    hash.addData(file.readAll());

    // Retrieve the SHA1 signature of the file
    QByteArray clienthash = hash.result();

    // If new version of CoX client in use, use below qDebug to print hash to console and replace in boolean check below
    // qDebug()<<(clienthash);

    // Compare SHA1 hashes
    if(clienthash != "\xFF""E0_\x1A\x84\x92\xB4\xCE\x84\xF7?\xFAk2JH\x8FM%")
    {
        ui->piggtool_output->appendPlainText("Wrong client version found... Stopping");
        return false;
    }

    ui->piggtool_output->appendPlainText("Correct client version found");
    return true;
}

bool SetUpData::copyPiggFiles()
{
    ui->piggtool_output->appendPlainText("Copying Files...");
    ui->progressBar->setEnabled(true);
    ui->buttonBox->setEnabled(false);
    QStringList source_file_list = {
        "bin.pigg",
        "geom.pigg",
        "geomBC.pigg",
        "geomV1.pigg",
        "geomV2.pigg",
        "mapsCities1.pigg",
        "mapsCities2.pigg",
        "mapsHazards.pigg",
        "mapsMisc.pigg",
        "mapsMissiong.pigg",
        "mapsTrials.pigg",
        "misc.pigg",
    };
    QDir sourceDir(ui->pigg_file_url->text()+"/piggs");
    QDir targetDir(QDir::currentPath()+"/data");
    int counter = 1;
    if(!sourceDir.exists()) // Stop if incorrect CoX directory selected
    {
        QMessageBox invalid_source_dir;
        invalid_source_dir.setText("Invalid CoX directory");
        invalid_source_dir.setInformativeText("Please ensure you select the main CoX directory");
        invalid_source_dir.setStandardButtons(QMessageBox::Ok);
        return false;
    }
    if(!targetDir.exists())  // If directory doesn't exist, create it
    {
        targetDir.mkdir(targetDir.absolutePath());
    }
    QFileInfoList fileInfoList = sourceDir.entryInfoList(source_file_list);
    foreach(QFileInfo fileInfo, fileInfoList)
    {
        // If file exists in targetDir already, delete it
        if(targetDir.exists(fileInfo.fileName()))
        {
            targetDir.remove(fileInfo.fileName());
        }
        QFile::copy(fileInfo.filePath(),targetDir.filePath(fileInfo.fileName())); // Copy files
        ui->progressBar->setValue(counter / fileInfoList.count() * 100);
        counter++;
        qDebug() << fileInfo.filePath() << targetDir.filePath(fileInfo.fileName()); // DEBUG
    }
    ui->icon_cox_directory->show();
    ui->piggtool_output->appendPlainText("File Copy Complete");
    return true;
}

//void SetUpData::pigg_dispatcher_wait_dialog()
//{
//    QMessageBox pigg_dispatcher_wait_msgbox;
//    pigg_dispatcher_wait_msgbox.setText("Stop Process?");
//    pigg_dispatcher_wait_msgbox.setInformativeText("This process is taking longer than expected, do you want to wait?");
//    pigg_dispatcher_wait_msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
//    pigg_dispatcher_wait_msgbox.setDefaultButton(QMessageBox::Yes);
//    int confirm = pigg_dispatcher_wait_msgbox.exec();
//    switch (confirm)
//    {
//    case QMessageBox::Yes:
//        break;
//    case QMessageBox::No:
//        emit quitPiggLoop();
//        break;
//    default:
//        break;
//    }
//}

// change to get maps_dir from settings.cfg
bool SetUpData::createDefaultDirectorys() // Creates default directories
{
    QString maps_dir = Helpers::getMapsDir();
    ui->label_create_directory->setEnabled(true);
    qDebug()<<"maps_dir: "<<maps_dir;
    QDir path(QDir::currentPath());
    for(const QString &map_name : g_map_names)
    {
        path.mkpath(maps_dir+"/"+map_name);
    }
    ui->icon_create_directory->show();
    ui->buttonBox->setEnabled(true);
    return true;
}

// Worker thread will signal once everything is done so main UI can re-check data.
void SetUpData::dataReady()
{
    emit dataSetupComplete();
}

//!@}

