/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *

 */

#include "MapTemplate.h"
#include "MapInstance.h"
MapTemplate::MapTemplate(const std::string &/*template_filename*/)
{
}
MapInstance * MapTemplate::get_instance(uint8_t game_server_id,uint32_t map_server_id)
{
    if(m_instances.size()==0)
    {
        m_instances.push_back(new MapInstance("City_00_01")); // should be replaced with proper cloning of map structure
        m_instances.back()->activate(THR_NEW_LWP|THR_JOINABLE|THR_INHERIT_SCHED,1);
        m_instances.back()->spin_up_for(game_server_id,map_server_id,1);
        m_instances.back()->start();
    }
    return m_instances.front();
}

size_t MapTemplate::num_instances()
{
    return m_instances.size();
}
