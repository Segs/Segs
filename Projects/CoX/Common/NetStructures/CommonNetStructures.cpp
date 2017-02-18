/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include "CommonNetStructures.h"
#include "Filesystem.h"
#include "CoXHash.h"
#include <QtCore/QString>
using namespace std;
void NetStructure::storeBitsConditional( BitStream &bs,int numbits,int bits )
{
    bs.StoreBits(1,bits!=0);
    if(bits)
        bs.StoreBits(numbits,bits);
}

int NetStructure::getBitsConditional( BitStream &bs,int numbits )
{
    if(bs.GetBits(1))
    {
        return bs.GetBits(numbits);
    }
    return 0;
}

void NetStructure::storePackedBitsConditional( BitStream &bs,int numbits,int bits )
{
    bs.StoreBits(1,bits!=0);
    if(bits)
        bs.StorePackedBits(numbits,bits);
}
void NetStructure::storeVector( BitStream &bs,glm::vec3 &vec )
{
    bs.StoreFloat(vec.x);
    bs.StoreFloat(vec.y);
    bs.StoreFloat(vec.z);
}
void NetStructure::storeVectorConditional(BitStream &bs, glm::vec3 &vec )
{
    storeFloatConditional(bs,vec.x);
    storeFloatConditional(bs,vec.y);
    storeFloatConditional(bs,vec.z);
}

void NetStructure::storeFloatConditional( BitStream &bs,float val )
{
    bs.StoreBits(1,val!=0.0);
    if(val!=0.0)
        bs.StoreFloat(val);
}

void NetStructure::storeFloatPacked( BitStream &bs,float val )
{
    bs.StoreBits(1,0);
    bs.StoreFloat(val);
}

int NetStructure::getPackedBitsConditional( BitStream &bs,int numbits )
{
    if(bs.GetBits(1))
    {
        return bs.GetPackedBits(numbits);
    }
    return 0;
}

void NetStructure::storeStringConditional( BitStream &bs,const QString &str )
{
    bs.StoreBits(1,str.size()>0);
    if(str.size()>0)
        bs.StoreString(str);
}

void NetStructure::storeTransformMatrix( BitStream &tgt,const Matrix4x3 &src )
{
    tgt.StoreBits(1,0); // no packed matrices for now
    tgt.StoreBitArray((uint8_t*)&src,12*4*8);
}

void NetStructure::storeTransformMatrix( BitStream &tgt,const TransformStruct &src )
{
    tgt.StoreBits(1,1); // partial
    tgt.StoreBits(1,src.v1_set ? 1:0);
    tgt.StoreBits(1,src.v2_set ? 1:0);
    tgt.StoreBits(1,src.v3_set ? 1:0);
    if(src.v1_set)
    {
        for(int i=0; i<3; i++)
            storeFloatPacked(tgt,src.v1[i]);
    }
    if(src.v2_set)
    {
        for(int i=0; i<3; i++)
            storeFloatPacked(tgt,src.v2[i]);
    }
    if(src.v3_set)
    {
        for(int i=0; i<3; i++)
            storeFloatPacked(tgt,src.v3[i]);
    }
}

void NetStructure::getTransformMatrix( BitStream &bs,Matrix4x3 &src )
{
    if(bs.GetBits(1))
        assert(!"PACKED ARRAY RECEIVED!");
    bs.GetBitArray((uint8_t*)&src,12*4*8);
}

void NetStructure::storeCached_Color( BitStream &bs,uint32_t col )
{
    uint32_t cache_idx=0;
    uint32_t prev_val=0;
    if(col && WorldData::instance()->colors().find_index(col,cache_idx,prev_val,false))
    {
        cache_idx+=1;
    }
    bs.StoreBits(1,(cache_idx||col==0));
    if(cache_idx||col==0)
    {
        bs.StorePackedBits(colorcachecount_bitlength,cache_idx);
    }
    else
    {
        bs.StoreBits(32,col);
    }
}

void NetStructure::storeCached_String( BitStream &bs,const QString & str )
{
    uint32_t cache_idx=0;
    uint32_t prev_val=0;
    if(str.size() && WorldData::instance()->strings().find_index(str,cache_idx,prev_val,false))
    {
        cache_idx+=1;
    }
    bs.StoreBits(1,(cache_idx||str.size()==0));
    if(cache_idx||str.size()==0)
        bs.StorePackedBits(stringcachecount_bitlength,cache_idx);
    else
        bs.StoreString(str);
}

uint32_t NetStructure::getCached_Color( BitStream &bs )
{
    bool in_hash= bs.GetBits(1);
    if(in_hash)
    {
        uint16_t hash_idx =bs.GetBits(colorcachecount_bitlength);
        uint32_t *kv = WorldData::instance()->colors().key_for_idx(hash_idx);
        if(kv)
            return *kv;
        return 0;
    }
    else
        return bs.GetBits(32);

    return 0;
}

QString NetStructure::getCached_String( BitStream &bs )
{
    std::ostringstream strm;
    QString tgt("");
    bool in_cache= bs.GetBits(1);
    if(in_cache)
    {
        int in_cache_idx = bs.GetPackedBits(stringcachecount_bitlength);
        QString *kv = WorldData::instance()->strings().key_for_idx(in_cache_idx);
        if(kv)
            tgt=*kv;
        return tgt;
    }
    else
        bs.GetString(tgt);
    return tgt;
}

