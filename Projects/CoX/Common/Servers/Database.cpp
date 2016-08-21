/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

// This module contains code to access our database where accounts and characters are stored
// segs Includes
#include "Database.h"

#include <cassert>
#include <sstream>
#include <string.h>

void PreparedArgs::add_param( const uint8_t *bytes,size_t len,bool binary )
{
    m_params.push_back(std::string((char *)bytes,len));
    m_lengths.push_back(len);
    m_formats.push_back(binary ? 1 : 0);
}

void PreparedArgs::add_param( const std::string &str )
{
    m_params.push_back(str);
    m_lengths.push_back(str.size());
    m_formats.push_back(0);
}

//FIXME : encoding ints in std::string - stinky code.
void PreparedArgs::add_param( uint16_t v )
{
    m_params.push_back(std::string((char *)&v,2));
    m_lengths.push_back(2);
    m_formats.push_back(1);
}
void PreparedArgs::add_param( uint32_t v )
{
    m_params.push_back(std::string((char *)&v,4));
    m_lengths.push_back(4);
    m_formats.push_back(1);
}
void PreparedArgs::add_param( uint64_t v )
{
    m_params.push_back(std::string((char *)&v,8));
    m_lengths.push_back(8);
    m_formats.push_back(1);
}
