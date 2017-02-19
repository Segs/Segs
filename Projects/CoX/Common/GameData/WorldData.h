/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "CoXHash.h"
#include "Common/GameData/costume_definitions.h"

#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>

#include <string>
#include <vector>
#include <fstream>
#include <map>

class QString;
class WorldData_impl
{
        Pallette_Data  m_supergroup_colors;
        CostumeSet_Data m_costume_store;
        StringHash      m_strings;
        ColorHash       m_colors;
        void            add_colors(const std::vector<ColorEntry_Data> &clr);
public:
        bool            read_costumes(const QString &src_filename);
        bool            read_colors(const QString &src_filename);
        void            fill_hashes();
const   StringHash &    strings() const { return m_strings; }
const   ColorHash &     colors() const { return m_colors; }
        StringHash &    strings() { return m_strings; }
        ColorHash &     colors() { return m_colors; }
};
typedef ACE_Singleton<WorldData_impl,ACE_Thread_Mutex> WorldData;
