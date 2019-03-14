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

MapInstance * MapTemplate::get_instance()
{
    if(m_instances.size()==0)
    {
        // TODO: here we should pass a pre-loaded map structure to an MapInstance
        uint16_t port_inc = s_template_id;
        m_instances.push_back(new MapInstance(m_map_filename,ListenAndLocationAddresses(m_base_loc,port_inc)));
        m_instances.back()->activate(THR_NEW_LWP|THR_JOINABLE|THR_INHERIT_SCHED,1);
        m_instances.back()->spin_up_for(m_game_server_id,m_map_server_id,s_template_id++);
        m_instances.back()->start(client_filename());
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
    return QString("maps/city_zones/%1/%1.txt").arg(map_desc_from_path);
}

QString MapTemplate::base_name() const
{
    int city_idx     = m_map_filename.indexOf('/')+1;
    int end_or_slash = m_map_filename.indexOf('/', city_idx);
    assert(city_idx != 0);
    return m_map_filename.mid(city_idx, end_or_slash == -1 ? -1 : m_map_filename.size() - end_or_slash).toLower();

}

size_t MapTemplate::num_instances()
{
    return m_instances.size();
}

//! @}
