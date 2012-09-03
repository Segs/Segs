/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once

typedef unsigned long int uint32;

struct NameTableEntry
{
    uint32 id;
    const char *name;
};

inline const char* LookupName(uint32 id, NameTableEntry *table)
{
    for(uint32 i = 0; table[i].name != 0; i++)
    {
        if (table[i].id == id)
            return table[i].name;
    }

    return "UNKNOWN OPCODES";
}

extern NameTableEntry OpcodeNames[];
