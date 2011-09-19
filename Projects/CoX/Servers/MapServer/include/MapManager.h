/*
 * Super Entity Game Server Project 
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

// Inclusion guards
#pragma once
#include "types.h"
#include <map>
#include <vector>
#include <string>
class MapTemplate;
/**
  \class MapManger
  \brief Central map server class, responsible for map template management.
*/
class MapManger
{
	std::map<u32,MapTemplate *> m_templates;
    size_t                      m_max_instances; // how many maps can we instantiate
public:
                    MapManger();
    bool            load_templates(const std::string &template_directory);
	MapTemplate *   get_template(u32 id);
    size_t          num_templates();
    size_t          max_instances();
};
