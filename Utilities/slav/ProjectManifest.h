/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef PROJECTMANIFEST_H
#define PROJECTMANIFEST_H

#include <QtCore/QString>
#include <set>

struct ServerDescriptor;

struct FileDescription {
    QString m_relativePath;
    mutable QString m_signature; // mutable since we can change this while not changing the sorted-by key
    bool operator==(const FileDescription &other) const {
        return m_relativePath==other.m_relativePath && m_signature==other.m_signature;
    }
    bool operator!=(const FileDescription &other) const {
        return m_relativePath!=other.m_relativePath || m_signature!=other.m_signature;
    }
    // used by std::set to order file descriptions
    bool operator<(const FileDescription &other) const { return m_relativePath<other.m_relativePath; }
};
class AppVersionManifest
{
public:
    const ServerDescriptor *m_associated_server = nullptr;
    std::set<FileDescription> m_files;
    QString m_version;
public:
    AppVersionManifest();
    void buildFromFileSystem(const QString &basePath);

    bool operator==(const AppVersionManifest &other) const;
    bool operator!=(const AppVersionManifest &other) const { return !(*this==other); }
};

bool loadFrom(AppVersionManifest &target, const QString &data);
bool storeTo(AppVersionManifest &target, QString &data);

AppVersionManifest calculateDelta(const AppVersionManifest &old_version,const AppVersionManifest &new_version);
#endif // PROJECTMANIFEST_H
