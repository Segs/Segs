/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Client.cpp 253 2006-08-31 22:00:14Z malign $
 */
#include <ace/OS.h>
#include "Costume.h"
#include "BitStream.h"

void CostumePart::serializeto( BitStream &bs ) const
{
    storeCached_String(bs,name_0);
    storeCached_String(bs,name_1);
    storeCached_String(bs,name_2);
    storeCached_Color(bs,m_colors[0]);
    storeCached_Color(bs,m_colors[1]);
    if(m_full_part)
    {
        storeCached_String(bs,name_3);
        storeCached_String(bs,name_4);
        storeCached_String(bs,name_5);
    }
}

void CostumePart::serializefrom( BitStream &bs )
{
    name_0=getCached_String(bs);
    name_1=getCached_String(bs);
    name_2=getCached_String(bs);
    m_colors[0]=getCached_Color(bs);
    m_colors[1]=getCached_Color(bs);
    if(m_full_part)
    {
        name_3=getCached_String(bs);
        name_4=getCached_String(bs);
        name_5=getCached_String(bs);
    }
}

void CostumePart::serializeto_charsel( BitStream &bs ) const
{
    // character selection needs to get part names as strings
    bs.StoreString(name_0);
    bs.StoreString(name_1);
    bs.StoreString(name_2);
    bs.StoreString(name_6);
    bs.StoreBits(32,m_colors[0]);
    bs.StoreBits(32,m_colors[1]);
}
