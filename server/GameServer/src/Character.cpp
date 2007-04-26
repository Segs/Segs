/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Client.cpp 253 2006-08-31 22:00:14Z malign $
 */
#include <ace/OS.h>
#include "Character.h"
#include "BitStream.h"

void Character::reset()
{
	m_level=0;
	m_name="EMPTY";
	m_archetype="EMPTY";
	m_origin="EMPTY";
	m_face_bits = 0;
	m_villain=0;
	m_bodyType=0;
	m_unkn1=m_unkn2=0.0f;
	m_unkn3=m_unkn4=0;
	m_mapName="EMPTY";
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
	m_bodyType = src.GetPackedBits(1);
	m_unkn1 =src.GetFloat();
	m_unkn2 =src.GetFloat();
	m_face_bits = src.GetBits(32);
	src.GetString(m_mapName);
	m_unkn3 = src.GetPackedBits(1);
	m_unkn4 = src.GetBits(32);
}
void Character::serializeto( BitStream &tgt) const
{
	tgt.StorePackedBits(1,m_level);
	tgt.StoreString(m_name);
	tgt.StoreString(m_archetype);
	tgt.StoreString(m_origin);
	tgt.StorePackedBits(1,m_villain);
	tgt.StorePackedBits(1,m_bodyType);
	tgt.StoreFloat(m_unkn1);
	tgt.StoreFloat(m_unkn2);
	tgt.StoreBits(32,m_face_bits);
	tgt.StoreString(m_mapName);
	tgt.StorePackedBits(1,m_unkn3);
	tgt.StorePackedBits(32,m_unkn4); // if != 0 UpdateCharacter is called
}

void Character::setName( const std::string &val )
{
	if(val.length()>0)
		m_name = val; 
	else
		m_name = "EMPTY";
}
