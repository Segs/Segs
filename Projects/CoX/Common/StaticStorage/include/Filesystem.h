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
#include <fstream>
#include <map>

#include "types.h"

#include "CoXHash.h"
#include "ReadableStructures.h"

class WorldData_impl
{
    ColorStorage m_supergroup_colors;
    CostumeStorage m_costumes;

public:
    bool read_costumes(const std::string &src_filename);
    bool read_colors(const std::string &src_filename);
    void fill_hashes()
    {

    }
};
typedef ACE_Singleton<WorldData_impl,ACE_Thread_Mutex> WorldData;