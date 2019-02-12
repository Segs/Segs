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

#include "MapTemplate.h"
#include "MapInstance.h"

using namespace SEGSEvents;

uint8_t MapTemplate::s_template_id = 1;

MapTemplate::MapTemplate(const QString &filename, uint8_t game_server_id, uint32_t map_server_id,
                         const ListenAndLocationAddresses &loc)
    : m_map_filename(filename), m_game_server_id(game_server_id), m_map_server_id(map_server_id), m_base_loc(loc)
{
}

MapInstance * MapTemplate::get_instance(bool is_mission_map, uint8_t mission_level_cap, uint8_t mission_layout)
{
    if(m_instances.size()==0)
    {
        // TODO: here we should pass a pre-loaded map structure to an MapInstance
        uint16_t port_inc = s_template_id;
        m_instances.push_back(new MapInstance(m_map_filename,ListenAndLocationAddresses(m_base_loc,port_inc)));
        m_instances.back()->activate(THR_NEW_LWP|THR_JOINABLE|THR_INHERIT_SCHED,1);
        m_instances.back()->spin_up_for(m_game_server_id,m_map_server_id,s_template_id++);
        if (is_mission_map)
        {
            // TODO: Update this to be able to pass back any mission map instance for a given mission type.
            // TODO: map filename needs to come from the mission_data from the map definition for each
            // TODO: Mission file paths probably need to be add to the instance so it can be sent for scene requests and such
            qDebug() << "Is mission map. Mission Level: " << mission_level_cap << " -- Layout: " << mission_layout;
            m_instances.back()->start(client_mission_filename(mission_level_cap, mission_layout));
        }
        else
        {
            m_instances.back()->start(client_filename());
        }
    }
    return m_instances.front();
}

void MapTemplate::shut_down_all()
{
    for(MapInstance * instance : m_instances)
    {
        if(instance->thr_count()>0)
        {
            instance->putq(Finish::s_instance->shallow_copy());
            instance->wait();
        }
        delete instance;
    }
}

QString MapTemplate::client_filename() const
{
    QString map_desc_from_path = base_name();
    // number suffix is likely the level limit (sewers_30 is levels 16-30)
    // each mission has a number of different layouts, each with a number of sub bins?
    // layout_01_01 - layout_01_04 for sewers_30, layout_02_01 - layout_02_04, etc.
    // "maps/missions/sewers/sewers_15/sewers_15_layout_01_01"
    // "maps/missions/sewers/sewers_30/sewers_30_layout_02_04"
    // some folders have audio.
    return QString("maps/city_zones/%1/%1.txt").arg(map_desc_from_path);
}

QString MapTemplate::client_mission_filename(uint8_t mission_level_cap, uint8_t mission_layout)
{
    QString map_desc_from_path = mission_base_name(mission_level_cap, mission_layout);
    
    int name_end_idx = map_desc_from_path.indexOf("_");
    QString base_map_name = map_desc_from_path.mid(0, name_end_idx);
    qInfo() << "Base Map Name: " << base_map_name;
    return QString("maps/missions/%1/%1_%2/%3.txt").arg(base_map_name).arg(mission_level_cap).arg(map_desc_from_path).toLower();
}

QString MapTemplate::base_name() const
{
    int city_idx     = m_map_filename.indexOf('/')+1;
    int end_or_slash = m_map_filename.indexOf('/', city_idx);
    assert(city_idx != 0);
    return m_map_filename.mid(city_idx, end_or_slash == -1 ? -1 : m_map_filename.size() - end_or_slash).toLower();

}

QString MapTemplate::mission_base_name(uint8_t mission_level_cap, uint8_t mission_layout)
{
    return QString("%1_%2_layout_01_01").arg(base_name().replace(QString("Mission_"), QString(), Qt::CaseInsensitive)).arg(mission_level_cap).toLower();
}

size_t MapTemplate::num_instances()
{
    return m_instances.size();
}

//! @}
