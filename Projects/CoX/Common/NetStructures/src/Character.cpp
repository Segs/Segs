/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Client.cpp 253 2006-08-31 22:00:14Z malign $
 */
#include <ace/OS.h>
#include "BitStream.h"
#include "Character.h"
#include "Costume.h"

Character::Character()
{
	m_multiple_costumes=false;
	m_current_costume_idx=0;
	m_current_costume_set=false;
	m_supergroup_costume=false;
	m_sg_costume=0;
	m_using_sg_costume=false;
}
void Character::reset()
{
	m_level=0;
	m_name="EMPTY";
	m_archetype="EMPTY";
	m_origin="EMPTY";
	m_villain=0;
	m_mapName="EMPTY";

	m_multiple_costumes=false;
	m_current_costume_idx=0;
	m_current_costume_set=false;
	m_supergroup_costume=false;
	m_sg_costume=0;
	m_using_sg_costume=false;

}

bool Character::isEmpty()
{
	return (0==ACE_OS::strcasecmp(m_name.c_str(),"EMPTY")&&(0==ACE_OS::strcasecmp(m_archetype.c_str(),"EMPTY")));
}

void Character::serializefrom( BitStream &src)
{
	m_level = src.GetPackedBits(1);
	src.GetString(m_name);
	src.GetString(m_archetype);
	src.GetString(m_origin);
	m_unkn1 =src.GetFloat();
	m_unkn2 =src.GetFloat();
	src.GetString(m_mapName);
	u32 unkn3 = src.GetPackedBits(1);
	//u32 unkn4 = src.GetBits(32);
}
void Character::serializeto( BitStream &tgt) const
{
	tgt.StorePackedBits(1,m_level);
	tgt.StoreString(m_name);
	tgt.StoreString(m_archetype);
	tgt.StoreString(m_origin);

	//tgt.StorePackedBits(1,m_villain);
	tgt.StoreString(m_mapName);
	tgt.StorePackedBits(1,m_unkn3);
	//tgt.StorePackedBits(32,m_unkn4); // if != 0 UpdateCharacter is called
}

void Character::setName( const std::string &val )
{
	if(val.length()>0)
		m_name = val; 
	else
		m_name = "EMPTY";
}

bool Character::serializeFromDB( u64 user_id,u32 slot_index )
{
	return false;
}

void Character::GetCharBuildInfo(BitStream &src)
{
	src.GetString(m_archetype);
	src.GetString(m_origin);
    PowerPool_Info primary,secondary;
    primary.serializefrom(src);
    secondary.serializefrom(src);

	m_powers.push_back(primary); // primary_powerset power
	m_powers.push_back(secondary); // secondary_powerset power
	m_trays.serializefrom(src);
}

void Character::serializetoCharsel( BitStream &bs )
{
	bs.StorePackedBits(1,m_level);
	bs.StoreString(m_name);
	bs.StoreString(m_archetype);
	bs.StoreString(m_origin);
	ACE_ASSERT(m_costumes.size()>0);
	m_costumes[m_current_costume_set]->storeCharsel(bs);
	bs.StoreString(m_mapName);
	bs.StorePackedBits(1,rand());
}

Costume * Character::getCurrentCostume()
{
	ACE_ASSERT(m_costumes.size()>0);
	if(m_current_costume_set)
		return m_costumes[m_current_costume_idx];
	else
		return m_costumes[0];
}

void Character::serialize_all_costumes( BitStream &bs ) const
{
	bs.StoreBits(1,m_current_costume_set);
	if(m_current_costume_set)
	{
		bs.StoreBits(32,m_current_costume_idx);
		bs.StoreBits(32,m_costumes.size());
	}
	bs.StoreBits(1,m_multiple_costumes);
	if(m_multiple_costumes)
	{
		for(size_t idx=0; idx<=m_costumes.size(); idx++)
		{
			m_costumes[idx]->serializeto(bs);
		}
	}
	else
	{
		m_costumes[m_current_costume_idx]->serializeto(bs);
	}
	bs.StoreBits(1,m_supergroup_costume);
	if(m_supergroup_costume)
	{
		m_sg_costume->serializeto(bs);
		bs.StoreBits(1,m_using_sg_costume);
	}
}
void Character::DumpPowerPoolInfo( const PowerPool_Info &pool_info )
{
	for(int i=0; i<3; i++)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    Pool_id[%d]: 0x%08x\n"),i,pool_info.id[i]));
	}
}
void Character::DumpBuildInfo()
{
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    class: %s\n"),m_class_name.c_str()));
	ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%I    origin: %s\n"),m_origin_name.c_str()));
	DumpPowerPoolInfo(m_powers[0]);
	DumpPowerPoolInfo(m_powers[1]);
}
