/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once
#include <string>
#include <vector>
#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>
#include "types.h"
#include "CoXHash.h"

class ColorStorage
{
    struct ColorEntry
    {
        float r,g,b;
    };
    std::vector<ColorEntry> m_colors;
public:
    static u32 color_to_4ub(const ColorEntry &e)
    {
        return ((u32)e.r) | (((u32)e.g)<<8) | (((u32)e.b)<<16) | (0xFF<<24);
    }

};

class WorldData_impl
{
public:
    bool read_costume_part_names(const std::string &src_filename);
    bool read_costume_part_color(const std::string &src_filename);
};
typedef ACE_Singleton<WorldData_impl,ACE_Thread_Mutex> WorldData;