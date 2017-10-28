/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include "CommonNetStructures.h"
#include "Common/GameData/CoXHash.h"
#include <QtCore/QString>
using namespace std;
void storeBitsConditional( BitStream &bs,int numbits,int bits )
{
    bs.StoreBits(1,bits!=0);
    if(bits)
        bs.StoreBits(numbits,bits);
}

int getBitsConditional( BitStream &bs,int numbits )
{
    if(bs.GetBits(1))
    {
        return bs.GetBits(numbits);
    }
    return 0;
}

void storePackedBitsConditional( BitStream &bs,int numbits,int bits )
{
    bs.StoreBits(1,bits!=0);
    if(bits)
        bs.StorePackedBits(numbits,bits);
}
void storeVector( BitStream &bs,glm::vec3 &vec )
{
    bs.StoreFloat(vec.x);
    bs.StoreFloat(vec.y);
    bs.StoreFloat(vec.z);
}
void storeVectorConditional(BitStream &bs, glm::vec3 &vec )
{
    storeFloatConditional(bs,vec.x);
    storeFloatConditional(bs,vec.y);
    storeFloatConditional(bs,vec.z);
}

void storeFloatConditional( BitStream &bs,float val )
{
    bs.StoreBits(1,val!=0.0);
    if(val!=0.0)
        bs.StoreFloat(val);
}

void storeFloatPacked( BitStream &bs,float val )
{
    bs.StoreBits(1,0);
    bs.StoreFloat(val);
}

int getPackedBitsConditional( BitStream &bs,int numbits )
{
    if(bs.GetBits(1))
    {
        return bs.GetPackedBits(numbits);
    }
    return 0;
}

void storeStringConditional( BitStream &bs,const QString &str )
{
    bs.StoreBits(1,str.size()>0);
    if(str.size()>0)
        bs.StoreString(str);
}

void storeTransformMatrix( BitStream &tgt,const Matrix4x3 &src )
{
    tgt.StoreBits(1,0); // no packed matrices for now
    tgt.StoreBitArray((uint8_t*)&src,12*4*8);
}

void storeTransformMatrix( BitStream &tgt,const TransformStruct &src )
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

void getTransformMatrix( BitStream &bs,Matrix4x3 &src )
{
    if(bs.GetBits(1))
        assert(!"PACKED ARRAY RECEIVED!");
    bs.GetBitArray((uint8_t*)&src,12*4*8);
}


