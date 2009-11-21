/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */
#include "CommonNetStructures.h"
#include "Filesystem.h"
#include "CoXHash.h"
std::vector<std::string> NetStructure::stringcache;
std::vector<u32>	NetStructure::colorcache;

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

void NetStructure::storeStringConditional( BitStream &bs,const string &str )
{
	bs.StoreBits(1,str.size()>0);
	if(str.size()>0)
		bs.StoreString(str);
}

void NetStructure::storeTransformMatrix( BitStream &tgt,const Matrix4x3 &src )
{
	tgt.StoreBits(1,0); // no packed matrices for now
	tgt.StoreBitArray((u8*)&src,12*4*8);
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
			storeFloatPacked(tgt,src.v1.v[i]);
	}
	if(src.v2_set)
	{
		for(int i=0; i<3; i++)
			storeFloatPacked(tgt,src.v2.v[i]);
	}
	if(src.v3_set)
	{
		for(int i=0; i<3; i++)
			storeFloatPacked(tgt,src.v3.v[i]);
	}
}

void NetStructure::getTransformMatrix( BitStream &bs,Matrix4x3 &src )
{
	if(bs.GetBits(1))
		ACE_ASSERT(!"PACKED ARRAY RECEIVED!");
	bs.GetBitArray((u8*)&src,12*4*8);
}

void NetStructure::storeCached_Color( BitStream &bs,u32 col )
{
    u32 cache_idx=0;
    u32 prev_val=0;
    if(col && ColorsHash::instance()->find_index(col,cache_idx,prev_val,false))
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

void NetStructure::storeCached_String( BitStream &bs,const std::string & str )
{
    u32 cache_idx=0;
    u32 prev_val=0;
    if(str.size() && PartsHash::instance()->find_index(str,cache_idx,prev_val,false))
    {
        cache_idx+=1;
    }
    bs.StoreBits(1,(cache_idx||str.size()==0));
	if(cache_idx||str.size()==0)
		bs.StorePackedBits(stringcachecount_bitlength,cache_idx);
	else
		bs.StoreString(str);
}

u32 NetStructure::getCached_Color( BitStream &bs )
{
	bool in_hash= bs.GetBits(1);
	if(in_hash)
	{
		u16 hash_idx =bs.GetBits(colorcachecount_bitlength);
        u32 *kv = ColorsHash::instance()->key_for_idx(hash_idx);
        if(kv)
            return *kv;
        return 0;
	}
	else
		return bs.GetBits(32);

	return 0;
}

std::string NetStructure::getCached_String( BitStream &bs )
{
	std::ostringstream strm;
	std::string tgt("");
	bool in_cache= bs.GetBits(1);
	if(in_cache)
	{
		int in_cache_idx = bs.GetPackedBits(stringcachecount_bitlength);
        std::string *kv = PartsHash::instance()->key_for_idx(in_cache_idx);
        if(kv)
            tgt=*kv;
        return tgt;
	}
	else
		bs.GetString(tgt);
	return tgt;
}

