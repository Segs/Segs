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

#include "ProjectDescriptor.h"
#include "CerealHelpers.h"

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <QtCore/QDebug>

CEREAL_CLASS_VERSION( ProjectDescriptor, 1 )
CEREAL_CLASS_VERSION( ServerDescriptor, 1 )
CEREAL_CLASS_VERSION( ProjectApp, 1 )
CEREAL_CLASS_VERSION( AppVersion, 1 )

template<class Archive>
static void serialize(Archive & archive, ServerDescriptor & m,uint32_t version)
{
    if(version==1) {
        archive(cereal::make_nvp("Name",m.m_name));
        archive(cereal::make_nvp("Location",m.m_location));
        archive(cereal::make_nvp("PresentationData",m.m_presentation_data));
        archive(cereal::make_nvp("ContentManifestUrl",m.m_additional_manifest));
        archive(cereal::make_nvp("Status",m.m_status));
        archive(cereal::make_nvp("MinVersion",m.m_lowest_supported_version));
        archive(cereal::make_nvp("MaxVersion",m.m_highest_supported_version));
    }
}
template<class Archive>
static void serialize(Archive & archive, AppVersion & m,uint32_t version)
{
    if(version==1) {
        archive(cereal::make_nvp("Os",m.m_os));
        archive(cereal::make_nvp("Presentation",m.m_version_number));
        archive(cereal::make_nvp("Changelog",m.m_change_log));
        archive(cereal::make_nvp("ContentManifestUrl",m.m_manifest_url));
    }
}
template<class Archive>
static void serialize(Archive & archive, ProjectApp & m,uint32_t version)
{
    if(version==1) {
        archive(cereal::make_nvp("Name",m.m_name));
        archive(cereal::make_nvp("Presentation",m.m_presentation_data));
        archive(cereal::make_nvp("Versions",m.m_versions));
        for(auto &val : m.m_versions) {
            val.m_app = &m;
        }
    }
}

template<class Archive>
static void serialize(Archive & archive, ProjectDescriptor & m,uint32_t version)
{
    if(version==1) {
        archive(cereal::make_nvp("ProjectName",m.m_project_name));
        archive(cereal::make_nvp("PresentationData",m.m_presentation_url));
        archive(cereal::make_nvp("Applications",m.m_apps));
        archive(cereal::make_nvp("Servers",m.m_servers));
        for(auto &val : m.m_apps) {
            val.m_project = &m;
        }
        for(auto &val : m.m_servers) {
            val.m_project = &m;
        }
    }
}


bool loadFrom(ProjectList & target, const QString &data)
{
    std::string strdat(data.toStdString());
    std::istringstream str_dat(strdat);
    try
    {
        cereal::JSONInputArchive input_arc(str_dat);
        input_arc(cereal::make_nvp("Projects",target.m_projects));
    }
    catch(...) {
        qDebug() << "Exception thrown while reading ProjectList";
        return false;
    }
    return true;
}

bool loadFrom(ProjectApp & target, const QString &data)
{
    std::string strdat(data.toStdString());
    std::istringstream str_dat(strdat);
    try
    {
        cereal::JSONInputArchive input_arc(str_dat);
        input_arc(target);
    }
    catch(...) {
        qDebug() << "Exception thrown while reading ProjectApp";
        return false;
    }
    return true;
}

//! @}
