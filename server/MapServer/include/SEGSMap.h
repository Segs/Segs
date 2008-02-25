/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: MapHandler.h 291 2006-10-12 10:52:55Z nemerle $
 */

// Inclusion guards
#pragma once
#ifndef SEGSMAP_H
#define SEGSMAP_H

#include "EntityStorage.h"

class SEGSMap
{
	string m_name;
public:
	SEGSMap(const string &name);
	EntityManager m_entities;

};

#endif // SEGSMAP_H
