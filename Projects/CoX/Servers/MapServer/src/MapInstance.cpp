/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#include "MapInstance.h"
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
