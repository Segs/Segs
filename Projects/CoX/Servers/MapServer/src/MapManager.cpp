/*
 * Super Entity Game Server Project 
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#include "MapManager.h"
#include "MapTemplate.h"
using namespace std;
MapManger::MapManger( ) : m_max_instances(2)
{
}
//! \brief Loads all templates available in given directory, will populate m_templates attribute
bool MapManger::load_templates( const std::string &/*template_directory*/ )
{
    //TODO: actually implement this, use boost path support ?
    // (template_directory / "tutorial.bin")
    m_templates[0] = new MapTemplate("tutorial.bin");
    return true;
}
//! \brief Retrieves template specified by it's id
MapTemplate * MapManger::get_template( u32 id )
{
    if(m_templates.find(id)==m_templates.end())
        return 0;
    return m_templates[id];
}

size_t MapManger::num_templates()
{
    return m_templates.size();
}

size_t MapManger::max_instances()
{
    return m_max_instances;
}
