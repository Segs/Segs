/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Client.cpp 253 2006-08-31 22:00:14Z malign $
 */
#include <ace/OS.h>
#include "CoXHash.h"

u32 JenkinsHash<std::string>::operator()(const std::string &val,u32 prev_val)
{
    return hash((const u8 *)val.c_str(),val.size(),prev_val);
}

template<class VALUE>
u32 CoXHashMap<VALUE>::find_index(const std::string &key, u32 &index_tgt, u32 &key_tgt, bool a5)
{
    int HashValue;
    int hash_index;
    u32 res;
    std::string tmp_key;
    HashValue = 0;
    tmp_key = key;
    if ( (m_flags & SINGLE_BYTE) )
    {
        if ( key.size() >= 0x1000 )
            return 0;
        for(size_t idx=0; idx<key.size(); idx++)
            tmp_key[idx]=toupper(tmp_key[idx]);
    }
    while ( 1 )
    {
        HashValue = hash(tmp_key,HashValue);
        hash_index = HashValue & (m_storage.size() - 1);
        if ( !HashValue )
        {
            //VfPrintfWrapper("Invalid HashValue %i generated while looking for index of key \"%s\"\n", 0, key);
            res = 2;
            break;
        }
        if (!m_storage[hash_index].key_hash || a5 && m_storage[hash_index].entry_flags & 1)
        {
            res = 0;
            break;
        }
        if ( HashValue != m_storage[hash_index].key_hash && !(m_storage[hash_index].entry_flags & 1) )
            continue;
        if ( !(m_flags & HAS_KEY_NAMES) || !(m_flags & CHECK_COLLISIONS) )
        {
            res = 1;
            break;
        }
        if(0==tmp_key.compare(m_storage[hash_index].stored_key)) // this does not handle wide characters ?
        {
            res = 1;
            break;
        }
        if ( !(m_flags & SKIP_CLASHES) )
        {
            //VfPrintfWrapper("Hash value conflict!  Both \"%s\" and \"%s\" produced a hash value of %i\n",v6->entries_array[hash_index].key,key,*((_DWORD *)&v6->str_tab + 4 * hash_index));
            res = 2;
            break;
        }
    }
    key_tgt = HashValue;
    index_tgt = hash_index;
    return res;
}
template
class CoXHashMap<std::string>;
template
class CoXGenericHashMap<u32,u32,IntCompare>;
