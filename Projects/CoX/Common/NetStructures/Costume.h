/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
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
    CostumePart(bool full_part,uint8_t part_type=0):m_type(part_type),m_full_part(full_part)
    {
        m_colors[0]=m_colors[1];
    }
    CostumePart(uint8_t type,const std::string &a,const std::string &b,const std::string &c,const std::string &d,uint32_t c1,uint32_t c2)
        :m_type(type),name_0(a),name_1(b),name_2(c),name_3(d)
    {
        m_colors[0]=c1;
        m_colors[1]=c2;
        }
    void serializeto(BitStream &bs) const;
    void serializefrom(BitStream &bs);
    void serializeto_charsel(BitStream &bs) const;
    uint8_t m_type; // arms/legs etc..
    std::string name_0,name_1,name_2,name_3,name_4,name_5,name_6;
    bool m_full_part;
    uint32_t m_colors[2];
};

class Costume : public NetStructure
{
public:
    float m_height;
    float m_physique;
    uint32_t a,b;
    bool m_non_default_costme_p;
    int m_num_parts;
    float m_floats[8];
    std::vector<CostumePart> m_parts;
    uint32_t m_body_type;
    Costume()
    {
        m_height=m_physique=0.0f;
        a=0;
        m_num_parts=0;
    }
    void storeCharselParts(BitStream &bs);
    void storeCharsel(BitStream &bs)
    {
        bs.StorePackedBits(1,m_body_type); // 0:male normal
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
protected:
    uint8_t		m_costume_type;
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
class CharacterCostume : public MapCostume
{
    uint8_t  m_slot_index;
    uint64_t m_character_id; //! Character to whom this costume belongs
public:

    static CharacterCostume NullCostume;

    uint8_t getSlotIndex() const { return m_slot_index; }
    void setSlotIndex(uint8_t val) { m_slot_index = val; }
    uint64_t getCharacterId() const { return m_character_id; }
    void setCharacterId(uint64_t val) { m_character_id = val; }

//    void serializeto(BitStream &tgt) const
//    {
//        tgt.StorePackedBits(1,(uint32_t)m_parts.size());
//        for(size_t i=0; i<m_parts.size(); i++)
//            m_parts[i].serializeto_charsel(tgt);
//    }
//    void serializefrom(BitStream &src)
//    {
//        uint32_t num_parts;
//        num_parts =  src.GetPackedBits(1);
//        for(size_t i=0; i<num_parts; i++)
//        {

//        }
//    }
    //////////////////////////////////////////////////////////////////////////
    // Database related
    //////////////////////////////////////////////////////////////////////////
    uint64_t	m_id;
};
