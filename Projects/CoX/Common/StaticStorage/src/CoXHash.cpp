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

template<class KEY,class VALUE,class COMPARE_FUNCTOR>
COMPARE_FUNCTOR CoXGenericHashMap<KEY, VALUE,COMPARE_FUNCTOR>::comp;

u32 JenkinsHash<std::string>::operator()(const std::string &val,u32 prev_val)
{
    return hash((const u8 *)val.c_str(),val.size(),prev_val);
}


template<class KEY,class VALUE>
JenkinsHash<KEY> CoxHashCommon<KEY, VALUE>::hash;


template<class VALUE>
u32 CoXHashMap<VALUE>::find_index(const std::string &key, u32 &index_tgt, u32 &key_tgt, bool a5) const
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
        if(0==key.compare(m_storage[hash_index].stored_key)) // this does not handle wide characters ?
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
template<class KEY,class VALUE,class COMPARE_FUNCTOR>
u32 CoXGenericHashMap<KEY, VALUE, COMPARE_FUNCTOR>::find_index( const KEY &needle,u32 &entry_idx,u32 &prev_val_out,bool a5 ) const
{
    u32 result;
    u32 prev_val;
    u32 h_idx=hash(needle,0);
    prev_val = h_idx;
    u32 entry_to_try = h_idx % m_storage.size();
    if(entry_to_try==0)
        return 2;
    while(true)
    {
        if(m_storage[entry_to_try].key_hash==0) // || a5 && m_storage[entry_to_try].entry_flags&1
        {
            result=0; // not in table
            break;
        }
        if(m_storage[entry_to_try].key_hash==prev_val) // if hashes are the same
        {
            // if!(entry_flags&1)
            if(0==comp(needle,m_storage[entry_to_try].stored_key)) // check the keys
            {
                result = 1; // found
                break;
            }
        }
        prev_val = hash(needle,prev_val);
        entry_to_try = prev_val % m_storage.size();
        if(!prev_val)
        {
            result = 2;
            break;
        }
    }
    entry_idx=entry_to_try;
    prev_val_out = prev_val;
    return result;
}
template
class CoXHashMap<std::string>;
template
class CoXGenericHashMap<u32,u32,IntCompare>;

