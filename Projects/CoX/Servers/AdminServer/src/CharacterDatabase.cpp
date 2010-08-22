/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

// segs includes
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <ace/OS_NS_time.h>
#include <ace/Log_Msg.h>

#include "CharacterDatabase.h"
#include "AccountInfo.h"
#include "AdminServer.h"
#include "Character.h"
#include "Costume.h"
using namespace std;
// UserToken,
bool CharacterDatabase::remove_character(AccountInfo *c,u8 slot_idx)
{
    stringstream query;
    DbResults results;
    ACE_ASSERT(c!=0);
    query << "DELETE FROM characters WHERE account_id="<<c->account_server_id()<<" AND slot_index="<<(u32)slot_idx<<";";
    if(!execQuery(query.str(),results))
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::remove_character %s failed. %s.\n"), query.str().c_str(),results.m_msg),false);
    return true;
}
bool CharacterDatabase::fill( AccountInfo *c )
{
	stringstream query;
	DbResults results;

	ACE_ASSERT(c&&c->account_server_id());
	query<<"SELECT * FROM accounts WHERE account_id="<<c->account_server_id();

	if(!execQuery(query.str(),results))
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query %s failed. %s.\n"), query.str().c_str(),results.m_msg),false);
	if(results.num_rows()!=1)
		return false;

	DbResultRow r=results.getRow(0);
	c->max_slots((u8)r.getColInt16("max_slots"));
	c->game_server_id((u64)r.getColInt64("id"));

	ACE_DEBUG((LM_INFO,"CharacterClient id: %i\n",c->account_server_id()));
	ACE_DEBUG((LM_INFO,"CharacterClient slots: %i\n",c->max_slots()));
	return true;
}
#define STR_OR_EMPTY(c) ((c!=0) ? c:"EMPTY")
#define STR_OR_VERY_EMPTY(c) ((c!=0) ? c:"")
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
	c->m_class_name = (STR_OR_EMPTY(r.getColString("archetype")));
	c->m_origin_name= (STR_OR_EMPTY(r.getColString("origin")));

    CharacterCostume *main_costume = new CharacterCostume;
    // appearance related.
    main_costume->m_body_type = r.getColInt32("bodytype"); // 0
    c->setMapName("Some map name"); // "V_City_00_01.txt" STR_OR_EMPTY(r.getColString("current_map"))
    c->setLastCostumeId(r.getColInt32("last_costume_id"));
    main_costume->setSlotIndex(0);
    main_costume->setCharacterId(r.getColInt32("id"));
    c->m_costumes.push_back(main_costume);
    return fill(main_costume);
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
	c->a = r.getColInt32("skin_color");
	query.str("");
	query<<"SELECT * FROM costume_part WHERE costume_id="<<r.getColInt32("id");
	// this will overwrite underlying object therefore 'r' will become useless
	if(!execQuery(query.str(),results))
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query %s failed. %s.\n"), query.str().c_str(),results.m_msg),false);
	for(size_t i=0; i<results.num_rows(); i++)
	{
		r=results.getRow(i);
		CostumePart part(true,r.getColInt32("part_type"));
		part.name_0=STR_OR_VERY_EMPTY(r.getColString("name_0"));
		part.name_1=STR_OR_VERY_EMPTY(r.getColString("name_1"));
		part.name_2=STR_OR_VERY_EMPTY(r.getColString("name_2"));
		part.name_3=STR_OR_VERY_EMPTY(r.getColString("name_3"));
		part.name_4=STR_OR_VERY_EMPTY(r.getColString("name_4"));
		part.name_5=STR_OR_VERY_EMPTY(r.getColString("name_5"));
		part.name_6=STR_OR_VERY_EMPTY(r.getColString("name_6"));
		part.m_colors[0]=r.getColInt32("color_0");
		part.m_colors[1]=r.getColInt32("color_1");
		c->m_parts.push_back(part);
	}
	return true;
/*
INSERT INTO costume_part(costume_id,part_type,name_0,name_1,name_2,name_3,color_0,color_1,color_2,color_3)
VALUES(,,'','','','',0,0,0,0);
*/
}

bool CharacterDatabase::CreateLinkedAccount( u64 auth_account_id,const std::string &username )
{
	stringstream query;
	DbResults results;
	ACE_ASSERT(auth_account_id>0);
	ACE_ASSERT(username.size()>2);

	query<<"INSERT INTO accounts  (account_id,max_slots) VALUES ("<<auth_account_id<<",2);";
	if(!execQuery(query.str(),results))
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query %s failed. %s.\n"), query.str().c_str(),results.m_msg),false);
	return true;
}

int CharacterDatabase::remove_account( u64 acc_serv_id )
{
	return 0;
}
//TODO: SQL String sanitization
bool CharacterDatabase::create( u64 gid,u8 slot,Character *c )
{
    stringstream query;
    DbResults results;
    ACE_ASSERT(gid>0);
    ACE_ASSERT(c);
    ACE_ASSERT(slot<8);
    s64 char_id = next_id("characters");
    if(char_id==-1)
        return false;

    query<<"INSERT INTO characters  (id,char_level,slot_index,account_id,char_name,archetype,origin,bodytype,current_map) VALUES ("\
                << char_id <<","<< c->m_level <<","<< u32(slot) <<",'"<< gid <<"','"<< c->m_name <<"','"\
                << c->m_class_name <<"','"<< c->m_origin_name <<"','"<< c->getCurrentCostume()->m_body_type <<"','"<< c->m_mapName <<"'"\
            <<");";
    if(!execQuery(query.str()))
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::create %s failed. %s.\n"), query.str().c_str(),results.m_msg),false);
    // create costume
    Costume *cst = c->getCurrentCostume();
    s64 cost_id = next_id("costume");
    if(cost_id==-1)
        return false;

    query.str("");
    query<<"INSERT INTO costume (id,character_id,costume_index,skin_color) VALUES ("
        << cost_id <<","<< char_id <<","<< u32(0) << ","<<u32(cst->a)<<");";
    if(!execQuery(query.str()))
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::create %s failed. %s.\n"), query.str().c_str(),results.m_msg),false);
    for(size_t idx=0; idx<cst->m_parts.size(); ++idx)
    {
        CostumePart &prt(cst->m_parts[idx]);
        query.str("");
        // prt.m_type is not filled by the client ?
        query<<"INSERT INTO costume_part (costume_id,part_type,name_0,name_1,name_2,name_3,color_0,color_1) VALUES ("\
                    << cost_id <<","<< idx <<",'"\
                    << prt.name_0 <<"','"<< prt.name_1 <<"','"<<prt.name_2 <<"','"<<prt.name_3 <<"',"\
                    << prt.m_colors[0] <<","<< prt.m_colors[1] << ");";
        fprintf(stderr,"%s\n",query.str().c_str());
        fflush(stderr);
        if(!execQuery(query.str()))
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::create %s failed. %s.\n"), query.str().c_str(),results.m_msg),false);

    }
    return true;
}
#ifdef DEFINED_ARRAYSIZE
#undef DEFINED_ARRAYSIZE
#undef ARRAYSIZE
#endif
