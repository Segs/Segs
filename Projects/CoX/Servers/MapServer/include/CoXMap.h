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
#include "EventProcessor.h"
class CoXMap : public EventProcessor
{
	string m_name;
public:
	CoXMap(const string &name);
	EntityManager       m_entities;
    void				dispatch(SEGSEvent *ev);
    SEGSEvent * 		dispatch_sync(SEGSEvent *ev);

};
