/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "MapManager.h"
#include "GameData/map_definitions.h"
#include "Components/Logging.h"
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
        //QString dirname = map_dir_visitor.next();
        map_dir_visitor.next();
        QString dirname = map_dir_visitor.fileInfo().baseName();

        if(dirname.contains("City_",Qt::CaseInsensitive) ||
            dirname.contains("Hazard_",Qt::CaseInsensitive) ||
            dirname.contains("Trial_",Qt::CaseInsensitive))
        {
            auto tpl = new MapTemplate(map_dir_visitor.fileInfo().filePath(), game_id, map_id, loc, false);
            m_templates[getMapIndex(tpl->base_name())] = tpl;
            m_name_to_template[tpl->client_filename()] = tpl;

            qInfo() << "Found map:" << map_dir_visitor.fileInfo().filePath();
        }

        qInfo() << "Directory: " << dirname;
        if (dirname.contains("Mission_", Qt::CaseInsensitive)) 
        {
            // TODO: Change this to create a single template for each mission type.
            qInfo() << "Found Mission Map: " << map_dir_visitor.fileInfo().filePath();
            // fileName: DefaultMapInstances/Mission_Sewers
            auto tpl = new MapTemplate(map_dir_visitor.fileInfo().filePath(), game_id, map_id, loc, true);
            m_templates[getMapIndex(tpl->mission_base_name())] = tpl;   // Sewers, Caves, etc
            // For missions, the client filename is only partial, as it still need the level and layouts appended to actually load
            m_name_to_template[tpl->client_filename()] = tpl;           // maps/missions/sewers, maps/missions/caves, etc.
        }
    }
    // (template_directory / "bin/tutorial.bin")
    return !m_templates.empty();
}

//! \brief Retrieves template specified by it's client-side path
MapTemplate * MapManager::get_template( QString id )
{
    // Mission templates are stored without all their variations, so need to just get the base mission type.
    if (id.contains("mission"))
    {
        int start = QString("maps/missions/").size();
        int index = id.indexOf(QString("/"), start, Qt::CaseInsensitive);
        id = id.mid(0, index);
    }
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
