/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */
#include <ace/OS.h>
#include "Filesystem.h"

bool WorldData_impl::read_costumes( const std::string &directory_path )
{
    ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Reading Costumes ... ") ));
    m_costumes.build_schema();
    bool res=m_costumes.read(directory_path+"costume.bin");
    if(res)
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Done.\n") ));
    else
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Failed!\n") ));
    return res;
}

bool WorldData_impl::read_colors( const std::string &directory_path )
{
    ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) Reading Supergroup colors ... ") ));
    m_supergroup_colors.build_schema();
    bool res=m_supergroup_colors.read(directory_path+"supergroupcolors.bin");
    if(res)
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Done.\n") ));
    else
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Failed!\n") ));
    return res;
}