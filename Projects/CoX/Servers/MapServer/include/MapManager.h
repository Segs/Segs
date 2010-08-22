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
class MapTemplate;
/**
  \class MapManger
  \brief Central map server class, responsible for map template management.
*/
class MapManger
{
	std::map<u32,MapTemplate *> m_templates;
public:
	MapTemplate get_template(u32 id);
};
class MapInstace;
class MapTemplate
{
	std::vector<MapInstance *> m_instances;
public:
	MapInstance *get_instance(); //! If there are no instances, starts a new one.
	size_t num_instances();
};
class MapInstance : public EventProcessor
{
	std::string m_name;
public:
	EntityManager	m_entities;
					MapInstance(const std::string &name);
	void			dispatch(SEGSEvent *ev);
	SEGSEvent * 	dispatch_sync(SEGSEvent *ev);
};
