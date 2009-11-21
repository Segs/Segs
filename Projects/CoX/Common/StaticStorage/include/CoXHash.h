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
#include <ace/Null_Mutex.h>
#include "types.h"

#define mix(a,b,c) \
{ \
    a -= b; a -= c; a ^= (c>>13); \
    b -= c; b -= a; b ^= (a<<8); \
    c -= a; c -= b; c ^= (b>>13); \
    a -= b; a -= c; a ^= (c>>12);  \
    b -= c; b -= a; b ^= (a<<16); \
    c -= a; c -= b; c ^= (b>>5); \
    a -= b; a -= c; a ^= (c>>3);  \
    b -= c; b -= a; b ^= (a<<10); \
    c -= a; c -= b; c ^= (b>>15); \
}
template<class V>
struct JenkinsHash
{
    static u32 hash(const u8 *k,u32 length,u32 initval)
    {
        register u32 a,b,c,len;

        /* Set up the internal state */
        len = length;
        a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
        c = initval;         /* the previous hash value */
        /*---------------------------------------- handle most of the key */
        while (len >= 12)
        {
            a += (k[0] +((u32)k[1]<<8) +((u32)k[2]<<16) +((u32)k[3]<<24));
            b += (k[4] +((u32)k[5]<<8) +((u32)k[6]<<16) +((u32)k[7]<<24));
            c += (k[8] +((u32)k[9]<<8) +((u32)k[10]<<16)+((u32)k[11]<<24));
            mix(a,b,c);
            k += 12; len -= 12;
        }
        /*------------------------------------- handle the last 11 bytes */
        c += length;
        switch(len)              /* all the case statements fall through */
        {
        case 11: c+=((u32)k[10]<<24);
        case 10: c+=((u32)k[9]<<16);
        case 9 : c+=((u32)k[8]<<8);
            /* the first byte of c is reserved for the length */
        case 8 : b+=((u32)k[7]<<24);
        case 7 : b+=((u32)k[6]<<16);
        case 6 : b+=((u32)k[5]<<8);
        case 5 : b+=k[4];
        case 4 : a+=((u32)k[3]<<24);
        case 3 : a+=((u32)k[2]<<16);
        case 2 : a+=((u32)k[1]<<8);
        case 1 : a+=k[0];
            /* case 0: nothing left to add */
        }
        mix(a,b,c);
        return c;
    }
    u32 operator()(const V &val,u32 prev_val)
    {
        return hash((const u8 *)&val,sizeof(V),prev_val);
    }
};
#undef mix

template<class KEY,class VALUE>
class CoxHashCommon
{
protected:
    struct HashEntry
    {
        HashEntry() : key_hash(0)
        {
        }
        u32 key_hash;
        KEY stored_key;
        VALUE stored_val;
        u32 entry_flags;
    };
    std::vector<HashEntry> m_storage;

    size_t max_size;
    size_t in_use;
    u32 m_flags;

    JenkinsHash<KEY> hash;
public:
    CoxHashCommon(){}
    void resize(size_t new_size)
    {
        u32 entry_idx;
        u32 prev_val;
        std::vector<HashEntry> old_entries;
        old_entries.assign(m_storage.begin(),m_storage.end());
        m_storage.clear();
        in_use=0;
        m_storage.resize(this->next_size(new_size));
        for(size_t idx=0; idx<old_entries.size(); ++idx)
        {
            if(old_entries[idx].key_hash==0) // || 0==old_entries[idx].entry_flags&1
                continue;
            if(find_index(old_entries[idx].stored_key,entry_idx,prev_val,1)==2)
                ACE_ASSERT(0);
            m_storage[entry_idx] = old_entries[idx];
            m_storage[entry_idx].key_hash = prev_val;
            in_use++;
        }
    }
    HashEntry *insert_entry(const KEY &k,VALUE v)
    {
        size_t watermark = (m_storage.size()*3)/4; // when we are filled upt 75% of our capacity
        u32 entry_idx=0;
        u32 prev_val=0;
        if((in_use+0)>=watermark || in_use>=m_storage.size())
        {
            u32 factor=1;
            if ( in_use >= watermark || in_use >= m_storage.size() - 1 )
                factor=2;
            resize(factor*m_storage.size());
        }
        if(this->find_index(k,entry_idx,prev_val,true))
        {
            return 0;
        }
        m_storage[entry_idx].stored_key = k;
        m_storage[entry_idx].stored_val = v;
        m_storage[entry_idx].key_hash = prev_val;

        return &m_storage[entry_idx];
    }
    KEY *key_for_idx(u32 idx)
    {
        idx-=1;
        if(idx >= 0 && idx < m_storage.size())
            if(m_storage[idx].key_hash!=0)
                return &m_storage[idx].stored_key;
        return 0;
    }
    virtual u32 find_index(const KEY &key, u32 &index_tgt, u32 &key_tgt, bool a5)=0;
    virtual u32 next_size(u32 sz)=0;

};
template<class VALUE>
class CoXHashMap : public CoxHashCommon<std::string,VALUE>
{
    enum{
        HAS_KEY_NAMES = 1,
        SKIP_CLASHES = 4,
        CHECK_COLLISIONS = 8,
        SINGLE_BYTE = 0x20,
    };
public:
    CoXHashMap(){};
    u32 find_index(const std::string &key, u32 &index_tgt, u32 &key_tgt, bool a5);
    u32 next_size(u32 sz)
    {
        if(sz==0)
            return 0;
        u32 bit=1<<31;
        int highest_bit;
        for(highest_bit=31; highest_bit>=0; --highest_bit)
        {
            if(sz&bit) // is the bit set ?
                break;
            bit>>=1;
        }
        if(sz!=bit) // size is not power of 2
            bit<<=1;
        return bit;
    }
    void init(size_t sz,u32 flags)
    {
        m_flags=flags;
        in_use=0;
        m_storage.resize(next_size(sz)); // reserve hash map entries
    }

};
struct IntCompare
{
    bool operator()(u32 a,u32 b)
    {
        return a!=b;
    }
};

template<class KEY,class VALUE,class COMPARE_FUNCTOR>
class CoXGenericHashMap : public CoxHashCommon<KEY,VALUE>
{
    COMPARE_FUNCTOR comp;
public:
    CoXGenericHashMap(){};
    u32 next_size(u32 sz)
    {
        size_t idx;
        static u32 prime_sizes[] = {
            11,     19,     37,     73,     109,    163,    251,    367,
            557,    823,    1237,   1861,   2777,   4177,   6247,   9371,
            14057,  21089,  31627,  47431,  71143,  106721, 160073, 240101,
            360163, 540217, 810343, 1215497,1823231,2734867,4102283,6153409,
            9230113,13845163,16313537,32452867
        };
        for(idx=0; idx<36; ++idx)
            if(prime_sizes[idx]>=sz)
                break;
        return prime_sizes[idx];
    }
    void init(size_t sz)
    {
        if(sz>0)
        {
            m_storage.resize(next_size(sz)); // reserve hash map entries
        }
        else
        {
            m_storage.resize(32);
        }
    }
    u32 find_index(const KEY &needle,u32 &entry_idx,u32 &prev_val_out,bool a5)
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
                entry_to_try = h_idx % m_storage.size();
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
};

typedef CoXHashMap<std::string> StringHash;
typedef CoXGenericHashMap<u32,u32,IntCompare> ColorHash_impl;

typedef ACE_Singleton<StringHash,ACE_Null_Mutex> PartsHash;
typedef ACE_Singleton<ColorHash_impl,ACE_Null_Mutex> ColorsHash;
