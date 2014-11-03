/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include <string>
#include <stdint.h>
#include <ace/Configuration.h>
#include <ace/Configuration_Import_Export.h>
class ACE_Configuration_Section_Key;
class ACE_INET_Addr;
class StringsBasedCfg : public ACE_Configuration_Heap
{
public:
    int get_integer_value_with_default(const ACE_Configuration_Section_Key& key,const ACE_TCHAR* name,uint16_t& value,uint16_t default_val);
    int get_string_value(const ACE_Configuration_Section_Key& key, const ACE_TCHAR* name, std::string& value,const std::string &default_val);
    int get_addr(const ACE_Configuration_Section_Key& key, const ACE_TCHAR* name, ACE_INET_Addr& value,const ACE_INET_Addr &default_val);
};
