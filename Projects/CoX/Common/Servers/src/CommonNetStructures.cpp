/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */
#include "CommonNetStructures.h"
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
	int cache_idx=0;
	if(col)
		ACE_ASSERT(!"Color cache lookup requested");
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
	int cache_idx=0;
	if(str.size())
		ACE_ASSERT(!"string cache lookup requested");
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
		u16 hash_key =bs.GetBits(colorcachecount_bitlength);
		return 0xFFFFFFFF; 
	}
	else
	{
		// update cache
		return bs.GetBits(32);
	}

	return 0;
}

std::string NetStructure::getCached_String( BitStream &bs )
{
	std::ostringstream strm;
	std::string tgt;
	bool in_cache= bs.GetBits(1);
	if(in_cache)
	{
		if(stringcache.size()==1)
		{
			return stringcache[0];
		}
		int in_cache_idx = bs.GetPackedBits(stringcachecount_bitlength);
		strm <<"Hash:"<<(int)in_cache_idx;
		tgt=strm.str();
		//tgt = stringcache[in_cache_idx];
		tgt.push_back(0);
	}
	else
	{
		bs.GetString(tgt);
		stringcache.push_back(tgt);
	}
	return tgt;
}

void CostumePart::serializeto( BitStream &bs ) const
{
	storeCached_String(bs,name_0);
	storeCached_String(bs,name_1);
	storeCached_String(bs,name_2);
	storeCached_Color(bs,m_colors[0]);
	storeCached_Color(bs,m_colors[1]);
	if(m_full_part)
	{
		storeCached_String(bs,name_3);
		storeCached_String(bs,name_4);
		storeCached_String(bs,name_5);
	}
}

void CostumePart::serializefrom( BitStream &bs )
{
	name_0=getCached_String(bs);
	name_1=getCached_String(bs);
	name_2=getCached_String(bs);
	m_colors[0]=getCached_Color(bs);
	m_colors[1]=getCached_Color(bs);
	if(m_full_part)
	{
		name_3=getCached_String(bs);
		name_4=getCached_String(bs);
		name_5=getCached_String(bs);
	}
}

void CostumePart::serializeto_charsel( BitStream &bs ) const
{
	// character selection needs to get part names as strings
	bs.StoreString(name_0);
	bs.StoreString(name_1);
	bs.StoreString(name_2);
	bs.StoreString(name_6);
	bs.StoreBits(32,m_colors[0]);
	bs.StoreBits(32,m_colors[1]);
}