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
#include "CostumeStructure.h"

class WorldData_impl
{
        ColorStorage    m_supergroup_colors;
        CostumeStorage  m_costume_store;
        StringHash      m_strings;
        ColorHash       m_colors;
        void            add_colors(const std::vector<BinReadable *> &clr);
public:
        bool            read_costumes(const std::string &src_filename);
        bool            read_colors(const std::string &src_filename);
        void            fill_hashes();
const   StringHash &    strings() const { return m_strings; }
const   ColorHash &     colors() const { return m_colors; }
        StringHash &    strings() { return m_strings; }
        ColorHash &     colors() { return m_colors; }
};
typedef ACE_Singleton<WorldData_impl,ACE_Thread_Mutex> WorldData;