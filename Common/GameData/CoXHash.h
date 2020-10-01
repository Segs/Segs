/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <cassert>
#include <string>
#include <vector>
#include <stdint.h>
#include <QtCore/QString>
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
    static uint32_t hash(const uint8_t *k,uint32_t length,uint32_t initval)
    {
        uint32_t a,b,c,len;

        /* Set up the internal state */
        len = length;
        a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
        c = initval;         /* the previous hash value */
        /*---------------------------------------- handle most of the key */
        while (len >= 12)
        {
            a += (k[0] +((uint32_t)k[1]<<8) +((uint32_t)k[2]<<16) +((uint32_t)k[3]<<24));
            b += (k[4] +((uint32_t)k[5]<<8) +((uint32_t)k[6]<<16) +((uint32_t)k[7]<<24));
            c += (k[8] +((uint32_t)k[9]<<8) +((uint32_t)k[10]<<16)+((uint32_t)k[11]<<24));
            mix(a,b,c);
            k += 12; len -= 12;
        }
        /*------------------------------------- handle the last 11 bytes */
        c += length;
        switch(len)              /* all the case statements fall through */
        {
        case 11: c+=((uint32_t)k[10]<<24);
        case 10: c+=((uint32_t)k[9]<<16);
        case 9 : c+=((uint32_t)k[8]<<8);
            /* the first byte of c is reserved for the length */
        case 8 : b+=((uint32_t)k[7]<<24);
        case 7 : b+=((uint32_t)k[6]<<16);
        case 6 : b+=((uint32_t)k[5]<<8);
        case 5 : b+=k[4];
        case 4 : a+=((uint32_t)k[3]<<24);
        case 3 : a+=((uint32_t)k[2]<<16);
        case 2 : a+=((uint32_t)k[1]<<8);
        case 1 : a+=k[0];
            /* case 0: nothing left to add */
        }
        mix(a,b,c);
        return c;
    }
    uint32_t operator()(const V &val,uint32_t prev_val) const
    {
        return hash((const uint8_t *)&val,sizeof(V),prev_val);
    }
};
//extern template struct JenkinsHash<std::string>;
#undef mix

template<class KEY,class VALUE>
class CoxHashCommon
{
protected:
    struct HashEntry
    {
        HashEntry() : key_hash(0),entry_flags(0)
        {
        }
        uint32_t key_hash;
        KEY stored_key;
        VALUE stored_val;
        uint32_t entry_flags;
    };
    std::vector<HashEntry> m_storage;

    size_t max_size;
    size_t in_use;
    uint32_t m_flags;

    static JenkinsHash<KEY> hash;
public:
    CoxHashCommon() : in_use(0),m_flags(0)
    {}
    virtual ~CoxHashCommon() {}
    virtual uint32_t find_index(const KEY &key, uint32_t &index_tgt, uint32_t &key_tgt, bool a5) const=0;
    virtual uint32_t next_size(uint32_t sz)=0;

    void resize(uint32_t new_size)
    {
        uint32_t entry_idx;
        uint32_t prev_val;
        std::vector<HashEntry> old_entries;
        std::swap(old_entries,m_storage);
        in_use=0;
        m_storage.resize(size_t(this->next_size(new_size)));
        for(size_t idx=0; idx<old_entries.size(); ++idx)
        {
            if(old_entries[idx].key_hash==0) // || 0==old_entries[idx].entry_flags&1
                continue;
            if(find_index(old_entries[idx].stored_key,entry_idx,prev_val,1)==2)
                assert(0);
            m_storage[entry_idx] = old_entries[idx];
            m_storage[entry_idx].key_hash = prev_val;
            in_use++;
        }
    }
    HashEntry *insert_entry(const KEY &k,VALUE v)
    {
        size_t watermark = (m_storage.size()*3)/4; // when we are filled upt 75% of our capacity
        uint32_t entry_idx=0;
        uint32_t prev_val=0;
        if(in_use>=watermark || in_use>=m_storage.size())
        {
            uint32_t factor=1;
            if( in_use >= watermark || in_use >= m_storage.size() - 1 )
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
        in_use++;
        return &m_storage[entry_idx];
    }
    const KEY *key_for_idx(int idx) const
    {
        idx-=1;
        if(idx >= 0 && idx < (int)m_storage.size())
            if(m_storage[idx].key_hash!=0)
                return &m_storage[idx].stored_key;
        return 0;
    }
    void init(uint32_t sz,uint32_t flags)
    {
        m_flags=flags;
        in_use=0;
        m_storage.resize(size_t(next_size(sz))); // reserve hash map entries
    }
};
template<class VALUE>
class CoXHashMap : public CoxHashCommon<QString,VALUE>
{
    enum{
        HAS_KEY_NAMES = 1,
        SKIP_CLASHES = 4,
        CHECK_COLLISIONS = 8,
        SINGLE_BYTE = 0x20,
    };
    typedef CoxHashCommon<QString,VALUE> super;
public:
    CoXHashMap(){}
    uint32_t find_index(const QString &key, uint32_t &index_tgt, uint32_t &key_tgt, bool a5) const;
    uint32_t next_size(uint32_t sz)
    {
        if(sz==0)
            return 0;
        uint32_t bit=1U<<31;
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
};
struct IntCompare
{
    bool operator()(uint32_t a,uint32_t b)
    {
        return a!=b;
    }
};

template<class KEY,class VALUE,class COMPARE_FUNCTOR>
class CoXGenericHashMap : public CoxHashCommon<KEY,VALUE>
{
protected:
    static COMPARE_FUNCTOR comp;
public:
    CoXGenericHashMap() = default;
    uint32_t next_size(uint32_t sz)
    {
        size_t idx;
        static uint32_t prime_sizes[] = {
            11,     19,     37,     73,     109,    163,    251,    367,
            557,    823,    1237,   1861,   2777,   4177,   6247,   9371,
            14057,  21089,  31627,  47431,  71143,  106721, 160073, 240101,
            360163, 540217, 810343, 1215497,1823231,2734867,4102283,6153409,
            9230113,13845163,16313537,32452867
        };
        for(idx=0; idx<36; ++idx)
            if(prime_sizes[idx]>=sz)
                return prime_sizes[idx];
        return prime_sizes[35];
    }
    void init(uint32_t sz)
    {
        if(sz>0)
        {
            this->m_storage.resize(size_t(next_size(sz))); // reserve hash map entries
        }
        else
        {
            this->m_storage.resize(32);
        }
    }
    uint32_t find_index(const KEY &needle,uint32_t &entry_idx,uint32_t &prev_val_out,bool a5) const;
};

typedef CoXHashMap<QString> StringHash;
typedef CoXGenericHashMap<uint32_t,uint32_t,IntCompare> ColorHash;
