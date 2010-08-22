/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: CoXMap.h 235 2010-08-22 16:27:50Z nemerle $
 */

// Inclusion guards
#pragma once

#include <map>
#include <vector>
#include "EntityStorage.h"
#include "EventProcessor.h"
class MapInstance : public EventProcessor
{
	std::string m_name;
public:
	EntityManager	m_entities;
					MapInstance(const std::string &name);
	void			dispatch(SEGSEvent *ev);
	SEGSEvent * 	dispatch_sync(SEGSEvent *ev);
};
