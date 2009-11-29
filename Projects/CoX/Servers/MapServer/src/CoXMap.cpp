/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: CoXMap.cpp $
 */

#include "CoXMap.h"
CoXMap::CoXMap( const string &name ) :m_name(name)
{

}

void CoXMap::dispatch( SEGSEvent *ev )
{
    
}

SEGSEvent * CoXMap::dispatch_sync( SEGSEvent *ev )
{
    return 0;
}
