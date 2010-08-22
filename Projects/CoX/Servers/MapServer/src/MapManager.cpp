/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: CoXMap.cpp 235 2010-08-22 16:27:50Z nemerle $
 */

#include "MapManager.h"
#include "MapManager.h"
using namespace std;
MapInstance::MapInstance( const string &name ) :m_name(name)
{

}

void MapInstance::dispatch( SEGSEvent *ev )
{

}

SEGSEvent * MapInstance::dispatch_sync( SEGSEvent *ev )
{
    return 0;
}
