/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#include "MapManager.h"

#include "MapTemplate.h"

#include <QString>
#include <QDir>
#include <QDirIterator>
#include <QHash>

static QHash<QString,int> s_map_name_to_id =
{
    {"City_00_01",0}, // Outbreak
    {"City_01_01",1}, // Atlas Park
    {"City_01_03",29}, // Galaxy City
};

using namespace std;
MapManager::MapManager( ) : m_max_instances(2)
{
}
//! \brief Loads all templates available in given directory, will populate m_templates attribute
bool MapManager::load_templates(const QString &template_directory, uint8_t game_id, uint32_t map_id,
                                const ListenAndLocationAddresses &loc)
{
    QDirIterator map_dir_visitor(template_directory);
    // (template_directory / "tutorial.bin")
    m_templates[0] = new MapTemplate("maps/City_Zones/City_01_01/City_01_01.txt", game_id, map_id, loc);
    return true;
}
//! \brief Retrieves template specified by it's id
MapTemplate * MapManager::get_template( uint32_t id )
{
    if(m_templates.find(id)==m_templates.end())
        return nullptr;
    return m_templates[id];
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
