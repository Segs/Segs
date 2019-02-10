/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
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
#include <QFileDialog>
#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <QApplication>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QStandardPaths>

SetUpData::SetUpData(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetUpData)
{
    ui->setupUi(this);
    QFont dejavu_font;
    dejavu_font.setFamily("DejaVu Sans Condensed");
    dejavu_font.setPointSize(12);
    ui->piggtool_output->setFont(dejavu_font);
    connect(ui->pigg_select_file,&QToolButton::clicked,this,&SetUpData::select_piggs_dir);
    connect(ui->copy_extract_button,&QPushButton::clicked,this,&SetUpData::check_client_version);
    connect(this,&SetUpData::fileCopyComplete,this,&SetUpData::pigg_dispatcher);
    connect(this,&SetUpData::callPiggWorker,this,&SetUpData::pigg_tool_worker);
    connect(this,&SetUpData::readyToCopy,this,&SetUpData::copy_piggs_files);
}

SetUpData::~SetUpData()
{
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

void SetUpData::select_piggs_dir()
{
    QString homedir = QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory);
    QString pigg_dir = QFileDialog::getExistingDirectory(this, tr("Select CoX Directory"),
                                                         homedir,
                                                         QFileDialog::ShowDirsOnly);
    ui->pigg_file_url->setText(pigg_dir);
    ui->copy_extract_button->setEnabled(true);
}

void SetUpData::check_client_version() // Generate SHA-1 hash of CoX.exe and compare against hash set below, checks if client version is correct.
{
    QString coxexe = ui->pigg_file_url->text().append("/CoX.exe");
    QCryptographicHash hash(QCryptographicHash::Sha1);
    QFile file(coxexe);

    if(file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"Generating SHA-1 Hash of CoX.exe";
        ui->piggtool_output->appendPlainText("Checking CoX client is correct version");
        hash.addData(file.readAll());
    }
    else
    {
        qDebug()<<"Cannot find or open CoX.exe";
        ui->piggtool_output->appendPlainText("Cannot find or open CoX.exe, did you pick the correct directory?");
    }

    // Retrieve the SHA1 signature of the file
    QByteArray clienthash = hash.result();

    // If new version of CoX client in use, use below qDebug to print hash to console and replace in boolean check below
    // qDebug()<<(clienthash);

    // Compare SHA1 hashes
    if(clienthash == "\xFF""E0_\x1A\x84\x92\xB4\xCE\x84\xF7?\xFAk2JH\x8FM%")
    {
        ui->piggtool_output->appendPlainText("Correct client version found");
        emit readyToCopy();
    }
    else
    {
        ui->piggtool_output->appendPlainText("Wrong client version found... Stopping");
    }
}

void SetUpData::copy_piggs_files()
{
    ui->piggtool_output->appendPlainText("Copying Files...");
    ui->progressBar->setEnabled(true);
    ui->buttonBox->setEnabled(false);
    QStringList sourceFileList = {
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
        return;
    }
    if(!targetDir.exists())  // If directory doesn't exist, create it
    {
        targetDir.mkdir(targetDir.absolutePath());
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList(sourceFileList);
    foreach(QFileInfo fileInfo, fileInfoList)
    {

        // If file exists in targetDir already, delete it

        if(targetDir.exists(fileInfo.fileName()))
        {
            targetDir.remove(fileInfo.fileName());
        }

        QFile::copy(fileInfo.filePath(),  // Copy files
                    targetDir.filePath(fileInfo.fileName()));

        ui->progressBar->setValue(counter / fileInfoList.count() * 100);
        counter++;
        qApp->processEvents();
    }
    ui->icon_cox_directory->show();
    ui->piggtool_output->appendPlainText("File Copy Complete");
    emit fileCopyComplete();
}
/*
 * Grabs a list of files then dispatches one to worker, when finished signal detected,
 * deletes pigg file and will send the next file.
 * Ensures only one pigg file is processed at a time and stops GUI freeze.
 */
void SetUpData::pigg_dispatcher()
{
    ui->piggtool_output->appendPlainText("Extracting Pigg Files...");
    ui->label_extract_pigg->setEnabled(true);
    ui->progressBar_piggtool->setEnabled(true);
    QStringList fileTypes;
    fileTypes << "*.pigg";
    QDir sourceDir(QDir::currentPath().append("/data"));
    QFileInfoList piggInfoList = sourceDir.entryInfoList(fileTypes);

    float counter = 0;
    foreach(QFileInfo fileInfo, piggInfoList)
    {
    //QString program = "piggtool -x data/bin.pigg";
        QString program = "piggtool -x " + fileInfo.absoluteFilePath();
        #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
                program.prepend("./");
        #endif
        emit callPiggWorker(program);
        QTimer timer;
        //timer.setSingleShot(true);
        QEventLoop loop;
        loop.connect(m_pigg_tool, SIGNAL(finished(int)), &loop, SLOT(quit())); // Couldn't get this to work on new signal/slot syntax
        loop.connect(&timer, SIGNAL(timeout()), this, SLOT(pigg_dispatcher_wait_dialog()));
        loop.connect(this, SIGNAL(quitPiggLoop()), &loop, SLOT(quit()));
        timer.start(30000); // 30 Second timer, will time out and prompt user if they want to wait or quit,
                          // if for some reason the event loop doesn't recieve a finished signal from pigg_tool_worker
        loop.exec();
        if(timer.isActive())
        {
            qDebug()<<"Completed";
        }
        else
        {
            qDebug()<<"Timeout";
        }
        float progress = counter / piggInfoList.count() * 100;
        ui->progressBar_piggtool->setValue(progress);
        counter++;
        //Delete pigg file once done with it
        QFile::remove(fileInfo.absoluteFilePath());
    }
    ui->progressBar_piggtool->setValue(100);
    ui->icon_extract_pigg->show();
    ui->piggtool_output->appendPlainText("Files Extracted");
    emit getMapsDir();
}

void SetUpData::pigg_dispatcher_wait_dialog()
{
    QMessageBox pigg_dispatcher_wait_msgbox;
    pigg_dispatcher_wait_msgbox.setText("Stop Process?");
    pigg_dispatcher_wait_msgbox.setInformativeText("This process is taking longer than expected, do you want to wait?");
    pigg_dispatcher_wait_msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    pigg_dispatcher_wait_msgbox.setDefaultButton(QMessageBox::Yes);
    int confirm = pigg_dispatcher_wait_msgbox.exec();
    switch (confirm)
    {
    case QMessageBox::Yes:
        break;
    case QMessageBox::No:
        emit quitPiggLoop();
        break;
    default:
        break;
    }
}

void SetUpData::pigg_tool_worker(QString program) // Processes pigg file from dispatcher, emits finished signal when done.
{
    m_pigg_tool = new QProcess(this);
    m_pigg_tool->start(program);

    if(m_pigg_tool->waitForStarted())
    {
        connect(m_pigg_tool,&QProcess::readyReadStandardError,this,&SetUpData::read_piggtool);
        connect(m_pigg_tool,&QProcess::readyReadStandardOutput,this,&SetUpData::read_piggtool);
    }
    else
    {
        qDebug() <<"Failed to process ";
    }
}

void SetUpData::read_piggtool()
{
    QByteArray out_err = m_pigg_tool->readAllStandardError();
    QByteArray out_std = m_pigg_tool->readAllStandardOutput();
    qDebug().noquote()<<QString(out_err);
    qDebug().noquote()<<QString(out_std);
    ui->piggtool_output->appendPlainText(out_err);
    ui->piggtool_output->appendPlainText(out_std);

}

void SetUpData::create_default_directory(QString maps_dir) // Creates default directories
{
    ui->label_create_directory->setEnabled(true);
    qDebug()<<"maps_dir: "<<maps_dir;
    QDir path(QDir::currentPath());
    for(const QString &map_name : g_map_names)
    {
        path.mkpath(maps_dir+"/"+map_name);
    }
    ui->icon_create_directory->show();
    ui->buttonBox->setEnabled(true);
    emit dataSetupComplete(maps_dir);
}

//!@}

