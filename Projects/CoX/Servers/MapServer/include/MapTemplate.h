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

#include <vector>
class MapInstance;
// TODO: instances should be uniquely identifiable, so getting a Client/Team specific instances is possible
class MapTemplate
{
	std::vector<MapInstance *> m_instances; // switch from vector to priority queue ?
public:
    MapTemplate(std::string &template_filename);
	MapInstance *   get_instance(); //! If there are no instances, starts a new one.
	size_t          num_instances();
};
// Generates instances based on some kind of schema file
class GeneratedMapTemplate : public MapTemplate
{
public:
    GeneratedMapTemplate();
};