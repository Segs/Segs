/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapViewer Projects/CoX/Utilities/MapViewer
 * @{
 */

#include "MapViewerApp.h"
#include "PiggTools.h"
#include "Lutefisk3D/Engine/Engine.h"
#include "Lutefisk3D/Core/Context.h"
#include "Lutefisk3D/Engine/Application.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtCore/QDebug>

#include <memory>

using namespace Urho3D;
static const QHash<QString,const char *> validHashes = {
    { "bin.pigg" , "CNMUJ8OZhEGW+/8E4oI01Q==" },
    { "geombc.pigg" , "3UdBHGSmHZuGQWYdF8iOXw==" },
    { "geom.pigg" , "Mj//QfOP9J03p1PSoPR6WQ==" },
    { "geomv1.pigg" , "oPsydmWTXyHVnLkhohWt9Q==" },
    { "geomv2.pigg" , "x7HPC5CndmsVhpqK9La3Bw==" },
    { "mapscities1.pigg" , "Xq9vj+kpAtHv6dDSaMHiIA==" },
    { "mapscities2.pigg" , "St+DEozSuH7IHQG98qMxMA==" },
    { "mapshazards.pigg" , "kdTQLoc6iPy+BXMtyQk0XA==" },
    { "mapsmisc.pigg" , "DoW3sTvd0P9Oyc3tFhLDuQ==" },
    { "mapsmissions.pigg" , "vAU5QXqpgONAWveDjwsnPA==" },
    { "mapstrials.pigg" , "7aUDWahTidcrtIq2yQ7NLg==" },
    { "misc.pigg" , "OdFFV2XUvLz4mGSOSrI+Ug==" },
    { "texenemies2.pigg" , "KXBrcbGp6QCW9/iufW8QTA==" },
    { "texenemies3.pigg" , "ZmC7itl+GKgVaNNazrMvqA==" },
    { "texenemies4.pigg" , "CJi/qiQro7htWIZya13zpQ==" },
    { "texenemies5.pigg" , "x2hmXf2tCeurN6ifNmz1hA==" },
    { "texenemies.pigg" , "gKDXDqROyrTv9s3Wgq6i1A==" },
    { "texfxguiitems.pigg" , "iEz1xs0ufVlJ08SqBpYgMg==" },
    { "texgui1.pigg" , "jBZYXXA30fr8oWNQz0eIFw==" },
    { "texgui2.pigg" , "LHf9IvwIo2UXxJaVUT08AQ==" },
    { "texmaps.pigg" , "H6K6N7MehaKVyAsJ/OL3gw==" },
    { "texmisc.pigg" , "8LcAvFvItUR3m4WNOacujA==" },
    { "texplayers2.pigg" , "O9fMRKVmlvVxPSFiSRbHhw==" },
    { "texplayersui.pigg" , "Ww4MWfbUHqAwUZpaa7da+g==" },
    { "texworldbc.pigg" , "5F0EurZqF0fHjirjbuOfmw==" },
    { "texworldbuildings.pigg" , "xwIXQn2yRW9L/6sCkdb4VA==" },
    { "texworldcz.pigg" , "Mlc4zdDXMG1x2QyKOBEiwA==" },
    { "texworld.pigg" , "hbjY7MX/Kiz8jCf64oEV9w==" },
    { "texworldsw.pigg" , "57jyhJztYmM2m+wlqwzn6A==" },
    { "texworldv1.pigg" , "Ss/xw1sVSZiRD9/gA+w+HA==" },
    { "texworldv2.pigg" , "vp5rDFXyzcfxXwci0eUxeQ==" },
};

bool dataDirIsValid(const QString &path)
{
    if(path.isEmpty())
        return false;
    if(!QFileInfo(path).exists())
        return false;
    return true;
}

bool isThatPiggRequired(const QString &file)
{
    QString filename = QFileInfo(file).fileName();
    if(!validHashes.contains(filename.toLower()))
        return false;
    return true;
}

bool checkPiggCrc(const QString &file, QProgressDialog &progress)
{
    QFile fi(file);
    if (!fi.open(QFile::ReadOnly))
        return false;

    QString filename = QFileInfo(file).fileName();
    assert(validHashes.contains(filename.toLower()));
    const QByteArray   req_hash(QByteArray::fromBase64(validHashes[filename.toLower()]));
    QCryptographicHash hasher(QCryptographicHash::Md5);
    bool               hashing_file = true;
    QByteArray         ba;
    progress.setLabelText(QString("Verifying %1").arg(filename));
    progress.setMaximum(fi.size());
    int processed = 0;
    while (hashing_file)
    {
        ba = fi.read(4096);
        processed += ba.size();
        hasher.addData(ba);
        progress.setValue(processed);
        qApp->processEvents();
        if (ba.size() != 4096)
            hashing_file = false;
    }
    qDebug() << '{' << filename.toLower() << ',' << hasher.result().toBase64() << '}';
    return req_hash == hasher.result();
}

QString askForPiggDirPath() {
    // ask for a path to pigg files
    bool we_are_piggless = true;
    QString dirpath;
    while(we_are_piggless)
    {
        dirpath=QFileDialog::getExistingDirectory(nullptr,"Point me to Your /piggs/ kind person");
        if(dirpath.isEmpty())
            break; // user aborted the selection
        QDir piggdir(dirpath);
        bool has_bin_pigg=piggdir.entryList().contains("bin.pigg");
        if(!has_bin_pigg)
            QMessageBox::critical(nullptr,"Error","Surprisingly there are no piggs in that pigg-sty");
        else
            we_are_piggless = false;
    }
    return dirpath;
}

bool explodizeThePiggs(const QStringList &piggfiles,const QString &tgtpath,QProgressDialog &progressdlg)
{
    int cnt=0;
    progressdlg.setMaximum(piggfiles.size());
    for(const QString &fl : piggfiles)
    {
        progressdlg.setLabelText(QString("Extracting %1").arg(fl));
        progressdlg.setValue(cnt++);
        qApp->processEvents();
        if(!unpackPiggFile(fl,tgtpath))
            return false;
    }
    return true;
}

bool acquireAndExplodePiggs()
{
    bool dialog_shown=false;
    QProgressDialog generic_progress_dlg;
    QStringList piggs_to_explodize;
    bool all_piggs_valid=false;
    while(!all_piggs_valid)
    {
        piggs_to_explodize.clear();
        QString pigg_dir_path=askForPiggDirPath();
        if(pigg_dir_path.isEmpty())
            break;
        generic_progress_dlg.show();
        generic_progress_dlg.setLabelText("Checking pigg files");
        qApp->processEvents();
        QDirIterator every_pigg(pigg_dir_path,QStringList{"*.pigg"},QDir::Files);
        while(every_pigg.hasNext())
        {
            QString file = every_pigg.next();
            if(!isThatPiggRequired(file))
                continue;
            if(!checkPiggCrc(file,generic_progress_dlg))
            {
                if(!dialog_shown) {
                    QMessageBox::critical(nullptr, "Error",
                                          QString("The '%1' looks strange ( does not match the known one )").arg(file));
                }
                else {
                    qWarning() << QString("The '%1' looks strange ( does not match the known one )").arg(file);
                }
            }
            else
                piggs_to_explodize << file;
        }
        if(piggs_to_explodize.size()==validHashes.size())
            all_piggs_valid=true;
    }
    if(!all_piggs_valid) {
        QMessageBox::critical(nullptr,"I give up","No piggs, no exploding.\nNo exploding, no viewing");
        return false;
    }

    QString dirpath;
    bool proper_exploding_location_selected=false;
    while(!proper_exploding_location_selected) {
        dirpath = QFileDialog::getExistingDirectory(nullptr,"Where shall I place the exploded pigg-bits?",".");
        if(dirpath.isEmpty())
            return false; // user aborted the selection
        if(QFileInfo(dirpath).isWritable())
            proper_exploding_location_selected = true;
    }

    if(!explodizeThePiggs(piggs_to_explodize,dirpath,generic_progress_dlg)) {
        QMessageBox::critical(nullptr,"No ignition!","A dud pigg, no explodey");
        return false;
    }
    QSettings our_settings(QSettings::IniFormat,QSettings::UserScope,"SEGS","MapViewer");
    our_settings.setValue("ExtractedDir",dirpath);
    return true;
}

bool checkDataFileAvailability()
{

    QSettings our_settings(QSettings::IniFormat,QSettings::UserScope,"SEGS","MapViewer");
    QString data_dir=our_settings.value("ExtractedDir",QString()).toString();
    if(dataDirIsValid(data_dir))
        return true;
    return false;
}

int main(int argc,char **argv)
{
    QApplication q_app(argc,argv);
    QApplication::setApplicationName("MapViewer");
    QApplication::setOrganizationName("SEGS");

    if(!checkDataFileAvailability()) {
        QMessageBox::critical(nullptr,"Oh woe is me!","The piggs are not unpacked !");
        if(!acquireAndExplodePiggs())
            return -1;
    }
    Urho3D::ParseArguments(argc, argv);
    auto context = std::make_shared<Context>();
    MapViewerApp app(context.get());
    app.Run();
    return 0;
}

//! @}
