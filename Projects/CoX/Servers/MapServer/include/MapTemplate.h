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
class MapTemplate
{
	std::vector<MapInstance *> m_instances;
public:
	MapInstance *get_instance(); //! If there are no instances, starts a new one.
	size_t num_instances();
};
