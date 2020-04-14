/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup slav Utilities/slav
 * @{
 */

#include "SLAVLogic.h"
#include "UpdaterDlg.h"
#include "ProjectManifest.h"
#include "ProjectDescriptor.h"
#include "ServerConnection.h"
#include "UpdaterDlg.h"
#include "FileSignatureWorker.h"

#include <cereal/cereal.hpp>
#include <QtCore/QCoreApplication>
#include <QtCore/QProcess>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QCryptographicHash>
#include <QThread>

namespace
{
    AppVersionManifest slav_manifest;
}

SLAVLogic::SLAVLogic(QObject *parent) : QObject(parent)
{
    m_serv_conn = new ServerConnection("http://segs.dev");
    m_ui_impl = new UpdaterDlg;
    m_current_manifest = new AppVersionManifest;

    connect(m_serv_conn,&ServerConnection::downloadProgress,m_ui_impl,&UpdaterDlg::onDownloadProgressed);
    connect(m_serv_conn,&ServerConnection::retrievedManifest,this,&SLAVLogic::onManifestReceived);
    connectUI();
}

void SLAVLogic::connectUI()
{
    connect(this,&SLAVLogic::needUpdate,m_ui_impl,&UpdaterDlg::onUpdateAvailable);
}

void SLAVLogic::start()
{
    if(needToOverwriteBaseInstall())
    {
        createBackupIfNeeded();
        copyFilesOverBase();
        //restartBaseInstall();
    }
    QFile slav_manifest_file("./slav.manifest");
    if(slav_manifest_file.open(QFile::ReadOnly))
    {
        loadFrom(slav_manifest,slav_manifest_file.readAll());
    }
    m_ui_impl->show();
    m_serv_conn->onRequestAppData();

}

void SLAVLogic::onManifestReceived(const QString &manifest_url,const QString &manifestData)
{
    AppVersionManifest pm;
    bool load_res = loadFrom(pm,manifestData);
    Q_UNUSED(load_res);
    // we've got a project manifest, perform work based on the manifest type.
    if(manifest_url.contains("slav.manifest"))
    {
        // Check if we need to update ourselves
        if(*m_current_manifest!=pm)
        {
            emit needUpdate(new AppVersionManifest(pm),"Changed stuff");
        }
    }

}

void SLAVLogic::onUpdateRequested(ServerDescriptor * manifest)
{
    if(manifest==nullptr)
    {
        // self - update request.
    }
}

bool SLAVLogic::needToOverwriteBaseInstall() {
    // case 1: we run from temp installation and CurrentChannel != BaseChannel || CurrentVersion>=BaseVersion
    QString apppath = qApp->applicationDirPath();
    QString workdir = QDir::currentPath();
    if(workdir.endsWith("install_temp")) {
        QDir parent_dir(workdir);
        if(!parent_dir.cdUp()) {
            qDebug() << "Cd up from temp install dir fails";
            return false;
        }
        AppVersionManifest parent_dir_manifest;
        if(QFile::exists(parent_dir.filePath("project.manifest"))) {
            QFile manifest_contents(parent_dir.filePath("project.manifest"));
            if(!manifest_contents.open(QFile::ReadOnly)) {
                qDebug() << "Cannot open parent dir manifest";
                return false;
            }
            if(!loadFrom(parent_dir_manifest,manifest_contents.readAll())) {
                qDebug() << "Failed to parse manifest contents";
                parent_dir_manifest.buildFromFileSystem(parent_dir.path());
            }
        }
    }
    AppVersionManifest current_manifest;
    QString manifest_path = workdir + "/project.manifest";
    if(QFile::exists(manifest_path)) {
        QFile manifest_contents(manifest_path);
        if(!manifest_contents.open(QFile::ReadOnly)) {
            qDebug() << "Cannot open parent dir manifest";
            return false;
        }
        if(!loadFrom(current_manifest,manifest_contents.readAll())) {
            qDebug() << "Failed to parse manifest contents";
        }
    }
    current_manifest.buildFromFileSystem(workdir);
    return false;
}

void SLAVLogic::createBackupIfNeeded()
{

}

void SLAVLogic::copyFilesOverBase()
{

}

void SLAVLogic::calculateHashes(AppVersionManifest *manifest)
{
    QThread* thread = new QThread(this);
    FileSignatureWorker* worker = new FileSignatureWorker();
    worker->setDataToProcess(manifest);
    worker->moveToThread(thread);
    connect(worker, &FileSignatureWorker::error, this, &SLAVLogic::onThreadError);
    connect(worker, &FileSignatureWorker::progress , this, &SLAVLogic::onHashingProgress);
    connect(thread, SIGNAL (started()), worker, SLOT (process()));
    connect(worker, SIGNAL (finished()), thread, SLOT (quit()));
    connect(worker, SIGNAL (finished()), worker, SLOT (deleteLater()));
    connect(thread, SIGNAL (finished()), thread, SLOT (deleteLater()));
    thread->start();
}

void SLAVLogic::onThreadError(QString v)
{
    qWarning() << "Thread error:" << v;
}

void SLAVLogic::onHashingProgress(QString chan, float percent)
{
    qDebug()<<"Hash progress " << chan << percent;
}

//! @}
