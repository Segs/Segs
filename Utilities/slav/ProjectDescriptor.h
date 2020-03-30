/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef PROJECTDESCRIPTOR_H
#define PROJECTDESCRIPTOR_H
#include <QtCore/QObject>
#include <QtCore/QString>

#include <vector>

struct ProjectDescriptor;
struct AppVersion
{
    QString            m_os;
    QString            m_version_number;
    QString            m_change_log;
    QString            m_manifest_url;
    struct ProjectApp *m_app;
};
struct ServerDescriptor
{
    enum eServerStatus
    {
        eOffline = 0,
        eOnline_Low,
        eOnline_Med,
        eOnline_High,
    };
    QString                  m_name;
    QString                  m_location;
    QString                  m_presentation_data;
    QString                  m_additional_manifest;
    int                      m_status;
    QString                  m_lowest_supported_version;
    QString                  m_highest_supported_version;
    const ProjectDescriptor *m_project;
};

struct ProjectApp
{
    QString                 m_name;
    QString                 m_presentation_data;
    std::vector<AppVersion> m_versions;
    const ProjectDescriptor *m_project;
};

struct ProjectDescriptor
{
    QString                       m_presentation_url;
    QString                       m_project_name;
    std::vector<ProjectApp>       m_apps;
    std::vector<ServerDescriptor> m_servers;

    QString displayName() const { return m_project_name; }
};
struct ProjectList
{
    std::vector<ProjectDescriptor> m_projects;
};
Q_DECLARE_METATYPE(ProjectList)
bool loadFrom(ProjectList &target, const QString &data);
bool loadFrom(ProjectApp &target, const QString &data);
#endif // PROJECTDESCRIPTOR_H
