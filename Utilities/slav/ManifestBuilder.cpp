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

#include "FileSignatureWorker.h"
#include "ProjectManifest.h"

#include <QFile>
#include <QFileInfo>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

bool calculateManifestForDirectory(const QString &dirpath, AppVersionManifest &manifest)
{
    if(!QFile::exists(dirpath)) {
        qCritical() << " Directory" << dirpath << "not accessible";
        return false;
    }
    manifest.buildFromFileSystem(dirpath);
    FileSignatureWorker fsw;
    fsw.setDataToProcess(&manifest);
    fsw.process();
    return true;
}
bool buildManifestDelta(const QString &original_manifest_path, const QString &new_directory,AppVersionManifest &delta)
{
    AppVersionManifest old_manifest;
    QFile              old_manifest_data(original_manifest_path);
    if(!old_manifest_data.open(QFile::ReadOnly)) {
        qCritical() << "Failed to open manifest:" << original_manifest_path <<':'<< old_manifest_data.errorString();
        return false;
    }
    if(!loadFrom(old_manifest, old_manifest_data.readAll())) {
        qCritical() << "Failed to load manifest:" << original_manifest_path;
        return false;
    }
    AppVersionManifest new_manifest;
    if(!calculateManifestForDirectory(new_directory, new_manifest)) {
        qCritical() << "Failed to create a manifest for:" << new_directory;
        return false;
    }
    delta = calculateDelta(old_manifest,new_manifest);
    return true;
}
void dumpManifest(AppVersionManifest &current_manifest) {
    QString             manifest_txt;
    storeTo(current_manifest, manifest_txt);
    printf("%s", qPrintable(manifest_txt));
}
/// Two work modes for now
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    AppVersionManifest current_manifest;
    QString dirpath = ".";

    if(qApp->arguments().size() > 1)
            dirpath = app.arguments()[1];

    if(app.arguments().size() > 2) {
        QString arg_1 = app.arguments()[1];
        QString arg_2 = app.arguments()[2];
        if(!arg_1.contains("project.manifest")) {
            qCritical() << "Path" << arg_1 << "does not contain project.manifest";
            return -1;
        }
        if(!QFile::exists(arg_1) || !QFile::exists(arg_2)) {
            qCritical() << "Either" << arg_1 << "or" << arg_2 << "don't exist";
            return -1;
        }
        if(!QFileInfo(arg_2).isDir()) {
            qCritical() << arg_2 << "is not a directory";
            return -1;
        }
        if(!buildManifestDelta(arg_1, arg_2,current_manifest))
        {
            qCritical() << "Failed to build delta manifest";
            return -1;
        }
    }
    else {
        if(!calculateManifestForDirectory(dirpath, current_manifest))
            return -1;
    }
    dumpManifest(current_manifest);
    return 0;
}

//! @}
