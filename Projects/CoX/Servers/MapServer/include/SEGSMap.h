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

#include "EntityStorage.h"

class SEGSMap
{
	string m_name;
public:
	SEGSMap(const string &name);
	EntityManager m_entities;

};
