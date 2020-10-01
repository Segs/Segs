/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "CoXHash.h"

template<class KEY,class VALUE,class COMPARE_FUNCTOR>
COMPARE_FUNCTOR CoXGenericHashMap<KEY, VALUE,COMPARE_FUNCTOR>::comp;

template<>
uint32_t JenkinsHash<QString>::operator()(const QString &val,uint32_t prev_val) const
{
    return hash((const uint8_t *)qPrintable(val),uint32_t(val.size()),prev_val);
}

template<class KEY,class VALUE>
JenkinsHash<KEY> CoxHashCommon<KEY, VALUE>::hash;

template<class VALUE>
uint32_t CoXHashMap<VALUE>::find_index(const QString &key, uint32_t &index_tgt, uint32_t &key_tgt, bool a5) const
{
    uint32_t HashValue;
    int hash_index;
    uint32_t res;
    QString tmp_key;
    HashValue = 0;
    tmp_key = key;
    if( (this->m_flags & SINGLE_BYTE) )
    {
        if( key.size() >= 0x1000 )
            return 0;
        tmp_key=tmp_key.toUpper();
    }
    while ( 1 )
    {
        HashValue = this->hash(tmp_key,HashValue);
        hash_index = HashValue & (int(this->m_storage.size()) - 1);
        if( !HashValue )
        {
            //("Invalid HashValue %i generated while looking for index of key \"%s\"\n", 0, key);
            res = 2;
            break;
        }
        const typename super::HashEntry &entry(this->m_storage[hash_index]);
        if( !entry.key_hash || (a5 && (entry.entry_flags & 1)) )
        {
            res = 0;
            break;
        }
        if( HashValue != entry.key_hash && !(entry.entry_flags & 1) )
            continue;
        if( !(this->m_flags & HAS_KEY_NAMES) || !(this->m_flags & CHECK_COLLISIONS) )
        {
            res = 1;
            break;
        }
        if(0==key.compare(entry.stored_key)) // this does not handle wide characters ?
        {
            res = 1;
            break;
        }
        if( !(this->m_flags & SKIP_CLASHES) )
        {
            //("Hash value conflict!  Both \"%s\" and \"%s\" produced a hash value of %i\n");
            res = 2;
            break;
        }
    }
    key_tgt = HashValue;
    index_tgt = hash_index;
    return res;
}

template<class KEY,class VALUE,class COMPARE_FUNCTOR>
uint32_t CoXGenericHashMap<KEY, VALUE, COMPARE_FUNCTOR>::find_index( const KEY &needle,uint32_t &entry_idx,uint32_t &prev_val_out,bool /*a5*/ ) const
{
    uint32_t result;
    uint32_t prev_val;
    uint32_t h_idx=this->hash(needle,0);
    prev_val = h_idx;
    uint32_t entry_to_try = h_idx % this->m_storage.size();
    if(entry_to_try==0)
        return 2;
    while(true)
    {
        if(this->m_storage[entry_to_try].key_hash==0) // || a5 && m_storage[entry_to_try].entry_flags&1
        {
            result=0; // not in table
            break;
        }
        if(this->m_storage[entry_to_try].key_hash==prev_val) // if hashes are the same
        {
            if(0==this->comp(needle,this->m_storage[entry_to_try].stored_key)) // check the keys
            {
                result = 1; // found
                break;
            }
        }
        prev_val = this->hash(needle,prev_val);
        entry_to_try = prev_val % this->m_storage.size();
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

//////////////////////////////////////////////////////////////////////////
// Implicit instantiations

// somehow this is needed on VC
template
JenkinsHash<std::string> CoxHashCommon<std::string, std::string>::hash;
template
JenkinsHash<uint32_t> CoxHashCommon<uint32_t, uint32_t>::hash;

template
class CoXHashMap<QString>;
template
class CoXGenericHashMap<uint32_t,uint32_t,IntCompare>;

//! @}
