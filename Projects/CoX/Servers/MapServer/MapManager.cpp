/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "MapManager.h"
#include "GameData/map_definitions.h"
#include "Logging.h"
#include "MapTemplate.h"

#include <QString>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QHash>

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
                                const ListenAndLocationAddresses &loc)
{
    qInfo() << "Searching for maps in:" << template_directory;

    QDirIterator map_dir_visitor(template_directory, QDir::Dirs|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (map_dir_visitor.hasNext())
    {
        QString dirname = map_dir_visitor.next();
        if(dirname.contains("City_",Qt::CaseInsensitive) ||
            dirname.contains("Hazard_",Qt::CaseInsensitive) ||
            dirname.contains("Trial_",Qt::CaseInsensitive))
        {
            auto tpl = new MapTemplate(map_dir_visitor.fileInfo().filePath(), game_id, map_id, loc);
            m_templates[getMapIndex(tpl->base_name())] = tpl;
            m_name_to_template[tpl->client_filename()] = tpl;

            qInfo() << "Found map:" << map_dir_visitor.fileInfo().filePath();
        }
    }
    // (template_directory / "bin/tutorial.bin")
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
