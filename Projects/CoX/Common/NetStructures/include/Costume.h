/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: CommonNetStructures.h 301 2006-12-26 15:50:44Z nemerle $
 */

#pragma once

#include <ace/Log_Msg.h>
#include <string>
#include <sstream>
#include "BitStream.h"
#include "CommonNetStructures.h"
class CostumePart : public NetStructure
{
public:
	// Part 6 : Hair
	// Part 0 : Lower Body
	// Part 1 : Upper Body
	// Part 2 : Head
	// Part 3 : Gloves
	// Part 4 : Boots
	CostumePart(bool full_part,size_t part_type=0):m_type(part_type),m_full_part(full_part)
	{
		m_colors[0]=m_colors[1];
	}
	CostumePart(int type,const string &a,const string &b,const string &c,const string &d,u32 c1,u32 c2)
		:m_type(type),name_0(a),name_1(b),name_2(c),name_3(d)
	{
		m_colors[0]=c1;
		m_colors[1]=c2;
	};
	void serializeto(BitStream &bs) const;
	void serializefrom(BitStream &bs);
	void serializeto_charsel(BitStream &bs) const;
	int m_type; // arms/legs etc..
	string name_0,name_1,name_2,name_3,name_4,name_5,name_6;
	bool m_full_part;
	u32 m_colors[2];
};

class Costume : public NetStructure
{
public:
    float m_height;
    float m_physique;
	u32 a,b;
	bool m_non_default_costme_p;
	int m_num_parts;
	float m_floats[8];
	vector<CostumePart> m_parts;
	u32 m_body_type;
    
    void storeCharselParts(BitStream &bs)
    {
        bs.StorePackedBits(1,m_num_parts);
        for(int costume_part=0; costume_part<m_num_parts;costume_part++)
            m_parts[costume_part].serializeto_charsel(bs);
    }
    void storeCharsel(BitStream &bs)
    {
        bs.StorePackedBits(3,m_body_type); // 0:male normal
        bs.StoreFloat(m_height);
        bs.StoreFloat(m_physique);
        bs.StoreBits(32,a); // rgb ?
    }
#if 0
0 male SM
1 female SF
2 bm BM
3 bf BF
4 huge SH
5,6 enemy EY
#endif
};
class MapCostume : public Costume
{
    bool costume_sends_nonquantized_floats;
    union
    {
        struct{
            float m_height;
            float m_physique;
        } split;
        float m_floats[30];
    };


protected:
    u8		m_costume_type;
    int		costume_type_idx_P;
    int		costume_sub_idx_P;
    void	GetCostume(BitStream &src);
    void	SendCommon(BitStream &bs) const;
public:
    ~MapCostume(){}
    MapCostume()
    {
    }
    void	clear_cache();
    void	serializefrom(BitStream &);
    void	serializeto(BitStream &bs) const;
    void	dump();
};
class CharacterCostume : public Costume
{
    u8  m_slot_index;
    u64 m_character_id; //! Character to whom this costume belongs
public:
    u8 getSlotIndex() const { return m_slot_index; }
    void setSlotIndex(u8 val) { m_slot_index = val; }
    u64 getCharacterId() const { return m_character_id; }
    void setCharacterId(u64 val) { m_character_id = val; }

    void serializeto(BitStream &tgt) const
    {
        tgt.StorePackedBits(1,(u32)m_parts.size());
        for(size_t i=0; i<m_parts.size(); i++)
            m_parts[i].serializeto_charsel(tgt);
    }
    void serializefrom(BitStream &src)
    {
        u32 num_parts;
        num_parts =  src.GetPackedBits(1);
        for(size_t i=0; i<num_parts; i++)
        {

        }
    }
    //////////////////////////////////////////////////////////////////////////
    // Database related
    //////////////////////////////////////////////////////////////////////////
    u64		m_id;
};
