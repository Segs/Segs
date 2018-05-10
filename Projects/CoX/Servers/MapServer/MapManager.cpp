/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "MapManager.h"
#include "Logging.h"
#include "MapTemplate.h"

#include <QString>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QHash>

static QHash<QString,int> s_map_name_to_id =
{
    {"city_00_01",0}, // Outbreak
    {"city_01_01",1}, // Atlas Park
    {"city_01_03",29}, // Galaxy City
};

using namespace std;

MapManager::MapManager( ) : m_max_instances(2)
{
}

MapManager::~MapManager()
{
    for(auto & v : m_templates)
    {
        delete v.second;
    }
}

//! \brief Loads all templates available in given directory, will populate m_templates attribute
bool MapManager::load_templates(const QString &template_directory, uint8_t game_id, uint32_t map_id,
                                const ListenAndLocationAddresses &loc, const MapConfig& map_config)
{
    qCDebug(logSettings) << "Starting the search for maps in" << template_directory;
    QDirIterator map_dir_visitor(template_directory, QDir::Dirs|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (map_dir_visitor.hasNext())
    {
        QString dirname = map_dir_visitor.next();
        if (dirname.contains("City_",Qt::CaseInsensitive))
        {
            auto tpl = new MapTemplate(map_dir_visitor.fileInfo().filePath(), game_id, map_id, loc, map_config);
            m_templates[s_map_name_to_id[tpl->base_name()]] = tpl;
            m_name_to_template[tpl->client_filename()] = tpl;
            qCDebug(logSettings) << "Detected a map"<<map_dir_visitor.fileInfo().filePath();
        }
    }
    // (template_directory / "tutorial.bin")
    return !m_templates.empty();
}

//! \brief Retrieves template specified by it's client-side path
MapTemplate * MapManager::get_template( QString id )
{
    if(m_name_to_template.find(id)==m_name_to_template.end())
        return nullptr;
    return m_name_to_template[id];
}

size_t MapManager::num_templates()
{
    return m_templates.size();
}

size_t MapManager::max_instances()
{
    return m_max_instances;
}

void MapManager::shut_down_all()
{
    for(std::pair<const uint32_t,MapTemplate *> &entry : m_templates)
    {
        entry.second->shut_down_all();
    }
}

//! @}
