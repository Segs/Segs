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
#include <ace/INET_Addr.h>
class StringsBasedCfg : public ACE_Configuration_Heap
{
public:
    int get_integer_value(const ACE_Configuration_Section_Key& key,const ACE_TCHAR* name,uint16_t& value,uint16_t default_val)
    {
        ACE_TString res;
        if(-1==ACE_Configuration_Heap::get_string_value(key,name,res))
        {
            value=default_val;
            return -1;
        }
        value = (uint16_t)strtoul(res.c_str(),0,0);
        return 0;
    }
    int get_string_value(const ACE_Configuration_Section_Key& key, const ACE_TCHAR* name, std::string& value,const std::string &default_val)
    {
        ACE_TString res;
        if(-1==ACE_Configuration_Heap::get_string_value(key,name,res))
        {
            value=default_val;
            return -1;
        }
        value=res.c_str();
        return 0;
    }
    int get_addr(const ACE_Configuration_Section_Key& key, const ACE_TCHAR* name, ACE_INET_Addr& value,const ACE_INET_Addr &default_val)
    {
        ACE_TString res;
        if(-1==ACE_Configuration_Heap::get_string_value(key,name,res))
        {
            value.set(default_val);
            return -1;
        }
        ACE_TString::size_type pos = res.rfind(':');
        if(ACE_TString::npos==pos) // invalid addr
        {
            value=default_val;
            return -1;
        }
        value.set((uint16_t)strtoul(res.substr(pos+1,ACE_TString::npos).c_str(),0,0),res.substr(0,pos).c_str());
        return 0;
    }
};
