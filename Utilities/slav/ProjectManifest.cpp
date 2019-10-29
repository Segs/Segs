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

#include "ProjectManifest.h"
#include "CerealHelpers.h"

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/set.hpp>
#include <set>

#include <QtCore/QDebug>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QDirIterator>
#include <cassert>

CEREAL_CLASS_VERSION( FileDescription, 1 )
CEREAL_CLASS_VERSION( AppVersionManifest, 1 )

/**
    Basic idea is to have 'base' project files and allow per-server 'mods' that add/override the base files.
*/
AppVersionManifest::AppVersionManifest()
{

}
static QString ifExistsGetContents(const QString &filePath,const QString &default_val) {
    if(QFile::exists(filePath)) {
        QFile channel_file(filePath);
        if(channel_file.open(QFile::ReadOnly)) {
            if(channel_file.size()>0 && channel_file.size()<512) { // stupid check for stupid cases
                return channel_file.readAll().trimmed();
            }
        }
    }
    return default_val;
}
void AppVersionManifest::buildFromFileSystem(const QString &basePath)
{
    // + - included in project manifest
    // - - not included in manifest.
    // schema is :
    // + project_dir/project.manifest - must contain at least Channel data for selected_channel
    // - project_dir/selected_channel - if missing 'release' is assumed
    // + project_dir/version - if missing '0.0.0' is assumed
    // - project_dir/*_backup - backup for given project channel,
    // - project_dir/install_temp - most recently downloaded update/channel change data
    // + project_dir/** project data
    QDir baseDir(basePath);
    m_version=ifExistsGetContents(baseDir.filePath("version"),"0.0.0");
    m_files.clear();

    QStringList files;
    QDirIterator it(basePath, QDir::Files,QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QString fpath = it.next();
        // add paths relative to basePath
        files << fpath.mid(basePath.size()+1);
    }
    QRegExp backupRegex(".*_backup");
    for(auto iter=files.begin(); iter!=files.end(); ) {
        if(iter->contains(backupRegex))
            iter = files.erase(iter);
        else
            ++iter;
    }
    files.removeAll("selected_channel");
    files.removeAll("install_temp");
    for(const QString &flp : files) {
        m_files.emplace(FileDescription {flp,""});
    }
}
bool AppVersionManifest::operator==(const AppVersionManifest & other) const
{
    if(m_version!=other.m_version)
        return false;
    if(m_files!=other.m_files)
        return false;
    return true;
}



template<class Archive>
static void serialize(Archive & archive, FileDescription & m,uint32_t version)
{
    if(version==1) {
        archive(cereal::make_nvp("Path",m.m_relativePath));
        archive(cereal::make_nvp("Signature",m.m_signature));
    }
}

template<class Archive>
static void serialize(Archive & archive, AppVersionManifest & m,uint32_t version)
{
    if(version==1) {
        archive(cereal::make_nvp("Filelist",m.m_files));
    }
}

bool loadFrom(AppVersionManifest &target, const QString &data)
{
    std::string strdat(data.toStdString());
    std::istringstream str_dat(strdat);
    try
    {
        cereal::JSONInputArchive input_arc(str_dat);
        input_arc(target);
    }
    catch(...) {
        qDebug() << "Exception thrown while reading scenedef";
        return false;
    }
    return true;

}
bool storeTo(AppVersionManifest &target, QString &data)
{
    std::ostringstream str_dat;
    try
    {
        cereal::JSONOutputArchive output_arc(str_dat);
        output_arc(target);
    }
    catch(...) {
        qDebug() << "Exception thrown while reading scenedef";
        return false;
    }
    data=QString::fromStdString(str_dat.str());
    return true;

}
///
/// \brief calculateDelta will build a 'delta'  manifest containing only 'new' files
/// \param old_version - manifest of the baseline app
/// \param new_version - manifest of the whole app, with mods
/// \return AppVersionManifest containing only new+changed files.
AppVersionManifest calculateDelta(const AppVersionManifest & old_version, const AppVersionManifest & new_version)
{
    std::set<FileDescription> original_fileset;
    std::set<FileDescription> new_fileset;
    original_fileset.insert(old_version.m_files.begin(),old_version.m_files.end());
    new_fileset.insert(new_version.m_files.begin(),new_version.m_files.end());
    AppVersionManifest dest;

    auto iter_last_original = original_fileset.end();

    // lazy ass code to select which files are to be returned.
    for(const FileDescription &desc : new_fileset) {
        auto iter = original_fileset.find(desc);
        if(iter==iter_last_original || iter->m_signature!=desc.m_signature) {
            dest.m_files.insert(desc);
        }
    }
    return dest;
}

//! @}
