/* 
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: AdminDatabase.cpp 267 2006-09-18 04:46:30Z nemerle $
 */

// segs includes
#include "CharacterDatabase.h"
#include <sstream>
#include <ace/OS_NS_time.h>
#include <ace/Log_Msg.h>
#include "CharacterClient.h"
#include "Character.h"

// UserToken,
int CharacterDatabase::AddCharacter(const std::string &username, const std::string &charname)
{
	// Who,
	// Awaiting nemerle's direction on this function
	return 0;
}

int CharacterDatabase::RemoveCharacter(char *username, char *charname)
{
	// Will implement later after AddCharacter function is complete
	return 0;
}
bool CharacterDatabase::fill( CharacterClient *c )
{
	stringstream query;
	DbResults results;

	ACE_ASSERT(c&&c->getId());
	query<<"SELECT * FROM accounts WHERE id="<<c->getId();

	if(!execQuery(query.str(),results))
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query %s failed. %s.\n"), query.str().c_str(),results.m_msg),false);
	if(results.num_rows()!=1)
		return false;

	DbResultRow r=results.getRow(0);
	c->setMaxSlots((u8)r.getColInt16("max_slots"));
	c->setGameServerAccountId((u64)r.getColInt64("id"));

	ACE_DEBUG((LM_INFO,"CharacterClient id: %i\n",c->getId()));
	ACE_DEBUG((LM_INFO,"CharacterClient slots: %i\n",c->getMaxSlots()));
	return true;
}
#define STR_OR_EMPTY(c) ((c!=0) ? c:"EMPTY")
bool CharacterDatabase::fill( Character *c)
{
	stringstream query;
	DbResults results;
	ACE_ASSERT(c&&c->getAccountId());
	query<<"SELECT * FROM characters WHERE account_id="<<c->getAccountId()<<" AND slot_index="<<(u16)c->getIndex();
	if(!execQuery(query.str(),results))
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query %s failed. %s.\n"), query.str().c_str(),results.m_msg),false);
	if(results.num_rows()==0)
	{
		c->reset(); // empty slot
		return true;
	}
	else if (results.num_rows()>1)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query returned wrong number of results. %s failed.\n"), query.str().c_str()),false);

	DbResultRow r=results.getRow(0);
	c->setLevel((u8)r.getColInt16("char_level"));
	c->setName(STR_OR_EMPTY(r.getColString("char_name")));
	c->setArchetype(STR_OR_EMPTY(r.getColString("archetype")));
	c->setOrigin(STR_OR_EMPTY(r.getColString("origin")));
	c->m_villain=r.getColBool("villain");
	// appearance related.	
	c->setBodyType(r.getColInt32("bodytype")); // 0
	c->setFace_bits(r.getColInt32("face_bits")); // this is the skin color
	c->setMapName(STR_OR_EMPTY(r.getColString("current_map"))); // "V_City_00_01.txt"
	c->m_unkn1=r.getColFloat("unkn1");//20.0f
	c->m_unkn2=r.getColFloat("unkn2");//30.0f;
	c->m_unkn3=r.getColInt32("unkn3");//1;//rand()|(rand()<<16); // can enter map ??
	c->m_unkn4=r.getColInt32("unkn4");//1;//0x7FFFFFFF^rand();
	c->setLastCostumeId(r.getColInt32("last_costume_id"));	

	c->m_costume=new CharacterCostume;
	c->m_costume->setSlotIndex(0);
	c->m_costume->setCharacterId(r.getColInt32("id"));
	return fill(c->m_costume);
}
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[1]))
#define DEFINED_ARRAYSIZE
#endif
bool CharacterDatabase::fill( CharacterCostume *c)
{
	stringstream query;
	DbResults results;
	ACE_ASSERT(c&&c->getCharacterId());
	query<<"SELECT * FROM costume WHERE character_id="<<c->getCharacterId()<<" AND costume_index="<<(u16)c->getSlotIndex();
	if(!execQuery(query.str(),results))
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query %s failed. %s.\n"), query.str().c_str(),results.m_msg),false);
	if(results.num_rows()!=1) // retry with the first one
	{
		query<<"SELECT * FROM costume WHERE character_id="<<c->getCharacterId()<<" AND costume_index=0";
		if(!execQuery(query.str(),results))
			ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query %s failed. %s.\n"), query.str().c_str(),results.m_msg),false);
	}
	DbResultRow r=results.getRow(0);
	c->m_face_used_ints = r.getColIntArray("arr7",c->m_face_ints_vec,ARRAYSIZE(c->m_face_ints_vec));
	c->m_used_floats = r.getColFloatArray("floats_14",c->m_floats_vec,ARRAYSIZE(c->m_floats_vec));
	query.str("");
	query<<"SELECT * FROM costume_part WHERE costume_id="<<r.getColInt32("id");
	// this will overwrite underlying object therefore 'r' will become useless
	if(!execQuery(query.str(),results))
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query %s failed. %s.\n"), query.str().c_str(),results.m_msg),false);
	for(size_t i=0; i<results.num_rows(); i++)
	{
		r=results.getRow(i);
		c->m_parts.push_back(CostumePart(r.getColInt32("part_type"),r.getColString("name_0"),
											r.getColString("name_1"),
											r.getColString("name_2"),
											r.getColString("name_3"),
											r.getColInt32("color_0"),
											r.getColInt32("color_1"),
											r.getColInt32("color_2"),
											r.getColInt32("color_3") ));
	}
	return true;
/*
INSERT INTO costume_part(costume_id,part_type,name_0,name_1,name_2,name_3,color_0,color_1,color_2,color_3)
VALUES(,,'','','','',0,0,0,0);
*/
}
#ifdef DEFINED_ARRAYSIZE
#undef DEFINED_ARRAYSIZE
#undef ARRAYSIZE
#endif