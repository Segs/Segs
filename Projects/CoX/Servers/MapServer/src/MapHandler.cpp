/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: MapHandler.cpp 319 2007-01-26 17:03:18Z nemerle $
 */

#include "MapHandler.h"
#include "MapPacket.h"
#include "GameProtocol.h"
#include "PacketCodec.h"
#include "ServerManager.h"
#include "MapServer.h"
#include "SEGSMap.h"
#include "Entity.h"
//#include "Character.h"
MapHandler::MapHandler(MapServer *srv):m_server(srv)
{
	m_client=0;
}
bool MapHandler::ReceivePacket(GamePacket *pak)
{

	ACE_ASSERT(m_server);
	if(ReceiveControlPacket((ControlPacket *)pak))
	{
		return true;
	}
	if(pak->getType()==COMM_CONNECT) // let's try not to overburden our servers
	{
		pak->dump();
		pktSC_Connected *res = new pktSC_Connected;
		m_proto->SendPacket(res);
		
		return true;
	}
	if(pak->m_opcode==9)
	{

		pktCS_SendEntity *in_pak = (pktCS_SendEntity *)pak;
		setClient(m_server->ClientExpected(getTargetAddr(),in_pak->m_cookie));
		if(in_pak->m_new_character)
		{
			m_client->setCharEntity(in_pak->get_new_character());
		}
		else
		{
			// get from db by id
		}
		in_pak->dump();
		//m_client->setHandler(this);
		ACE_ASSERT(m_client!=0);
		// pktSC_Connect is created inside HandleClientPacket
		m_proto->SendPacket(m_client->HandleClientPacket(in_pak));
		return true;
	}
	if(pak->m_opcode==4)
	{
		//m_proto->SendPacket(m_client->handleClientPacket(in_pak));
		pktSC_CmdShortcuts *res = new pktSC_CmdShortcuts;
		res->m_client=m_client;
		((pktSC_CmdShortcuts*)res)->m_num_shortcuts2=0;
		m_proto->SendPacket(res);
		return true;
	}
	if(pak->m_opcode==3)
	{
		pktMap_Server_SceneResp *res = new pktMap_Server_SceneResp;
		res->undos_PP=0;
		res->var_14=1;
		res->m_outdoor_map=1;//0;
		res->m_map_number=1;
		res->m_map_desc="maps/City_Zones/City_00_01/City_00_01.txt";
		res->current_map_flags=1; //off 1
		Vector3 positions[]=
		{
			Vector3(0,0,0),Vector3(0,0,0),Vector3(0,0,0),Vector3(0,0,0),
			Vector3(0,0,0),Vector3(0,0,0),Vector3(0,0,0),Vector3(0,0,0),
			Vector3(0,0,0),Vector3(0,0,0),Vector3(0,0,0),Vector3(0,0,0),
			Vector3(0,0,0),Vector3(0,0,0),
		};
		string names[]={
			"grpa2228","grp2544",
			"object_library/city_templates/map_2d_blackpoly/map_city_00_01","grp2555",
			"grp2561","grp2575",
			"grp2578","grp2582",
			"grpsound2583","grpsound2584",
			"grp2585","grp2586",
			"grp2587","grp2588",
		};
/*
		int ids[]=
		{
			17806, // 2228 -> def_id 17805, client subs 1
			18127,
			18248,
			18138,
			18190,
			18204,
			18218,
			18222,
			18223,
			18224,
			18225,
			18233,
			18234,
			18235,
		};
		for(int i=0; i<13; i++)
			res->m_refs.push_back(MapRef(ids[i],names[i],positions[i],Vector3(0.0,0.0,0.0)));
			res->m_crc.resize(res->m_trays.size());
		Matrix4x3 mat;
		for(size_t j=0; j<sizeof(Matrix4x3)/4; j++)
		{
			((float *)&mat.row1)[j]=0.0f;
		}
		mat.row1.vals.x = mat.row2.vals.y = mat.row3.vals.z= 1.0;
		res->ref_crc=0;
		res->ref_count=11;
*/
		res->unkn1=1;
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%d - %d - %d\n"),res->unkn1,res->undos_PP,res->current_map_flags));
		res->unkn2=1;
		m_proto->SendPacket(res);
		return true;
	}
	if(pak->m_opcode==5)
	{
		pktMap_Server_EntitiesResp * res = new pktMap_Server_EntitiesResp(m_client);
		//pktMap_Server_Connect* res = new pktMap_Server_Connect;
		Entity *pent = m_client->getCharEntity(); //new Entity;//EntityManager::CreatePlayer();
		if(pent)
		{
			pent->m_idx=0;
			pent->m_create=true;
			pent->var_129C=false;
			pent->m_type = 2; //PLAYER
			pent->m_create_player=true;
			pent->m_player_villain=false;
			pent->m_origin_idx=pent->m_class_idx=0;
			pent->m_selector1=false;
			pent->m_hasname = true;
			//pent->m_name ="Dummy001";
			pent->m_hasgroup_name=false;
			pent->m_pchar_things=false;
			pent->m_rare_bits = true;
//			pent->m_costume->m_costume_type=2; // npc costume for now

			m_client->getCurrentMap()->m_entities.m_entlist.push_back(pent);
		}

		pent = new MobEntity;
		pent->m_idx=1;
		pent->m_create=true;
		pent->might_have_rare=true;
		pent->var_129C=true; // will break entity-receiving loop
		m_client->getCurrentMap()->m_entities.m_entlist.push_back(pent);

		res->entReceiveUpdate=false;
//		res->m_resp=1;
		res->unkn1=false;
		res->m_num_commands=0;
		res->abs_time = (u32)time(NULL);
		res->unkn2=true; // default parameters for first flags

/*
*/
//		res->unkn3=false;
/*
		res->m_command_idx=0;
		res->dword_151B644=false;
		res->u1=1;
		res->u2=1;
		res->u3=0;
		res->u4=0;
*/
		m_proto->SendPacket(res);
		return true;
//		send_it=1;
	}
	// after successfuly receiving entities, client sends packet 6
	// it contains cookie, and a flag stating if the client has debugging console enabled
	if(pak->m_opcode==6)
	{
		pak->dump();
		m_proto->SendPacket(new pktSC_Connected);
		return true;
	}
	pak->dump();
	m_proto->SendPacket(new pktIdle);
	return false;
}
static void FillCommands()
{
	NetCommandManager *cmd_manager = NetCommandManagerSingleton::instance();
	{
		NetCommand::Argument arg1={1,NULL};
		NetCommand::Argument arg_1float={3,NULL};
		vector<NetCommand::Argument> args;
		args.push_back(arg1);
		vector<NetCommand::Argument> fargs;
		fargs.push_back(arg_1float);
		cmd_manager->addCommand(new NetCommand(9,"controldebug",args));
		cmd_manager->addCommand(new NetCommand(9,"nostrafe",args));
		cmd_manager->addCommand(new NetCommand(9,"alwaysmobile",args));
		cmd_manager->addCommand(new NetCommand(9,"repredict",args));
		cmd_manager->addCommand(new NetCommand(9,"neterrorcorrection",args));
		cmd_manager->addCommand(new NetCommand(9,"speed_scale",fargs));
		cmd_manager->addCommand(new NetCommand(9,"svr_lag",args));
		cmd_manager->addCommand(new NetCommand(9,"svr_lag_vary",args));
		cmd_manager->addCommand(new NetCommand(9,"svr_pl",args));
		cmd_manager->addCommand(new NetCommand(9,"svr_oo_packets",args));
		cmd_manager->addCommand(new NetCommand(9,"client_pos_id",args));
		cmd_manager->addCommand(new NetCommand(9,"atest0",args));
		cmd_manager->addCommand(new NetCommand(9,"atest1",args));
		cmd_manager->addCommand(new NetCommand(9,"atest2",args));
		cmd_manager->addCommand(new NetCommand(9,"atest3",args));
		cmd_manager->addCommand(new NetCommand(9,"atest4",args));
		cmd_manager->addCommand(new NetCommand(9,"atest5",args));
		cmd_manager->addCommand(new NetCommand(9,"atest6",args));
		cmd_manager->addCommand(new NetCommand(9,"atest7",args));
		cmd_manager->addCommand(new NetCommand(9,"atest8",args));
		cmd_manager->addCommand(new NetCommand(9,"atest9",args));
		cmd_manager->addCommand(new NetCommand(9,"predict",args));
		cmd_manager->addCommand(new NetCommand(9,"notimeout",args)); // unknown-10,argtype-1
		cmd_manager->addCommand(new NetCommand(9,"selected_ent_server_index",args));
		cmd_manager->addCommand(new NetCommand(9,"record_motion",args));

		cmd_manager->addCommand(new NetCommand(9,"time",fargs)); // unknown = 12
		cmd_manager->addCommand(new NetCommand(9,"timescale",fargs)); // unknown = 13
		cmd_manager->addCommand(new NetCommand(9,"timestepscale",fargs)); // unknown = 14
		cmd_manager->addCommand(new NetCommand(9,"pause",args)); 
		cmd_manager->addCommand(new NetCommand(9,"disablegurneys",args));
		cmd_manager->addCommand(new NetCommand(9,"nodynamiccollisions",args));
		cmd_manager->addCommand(new NetCommand(9,"noentcollisions",args));
		cmd_manager->addCommand(new NetCommand(9,"pvpmap",args)); // unknown 16
	}
}
void MapHandler::setClient(IClient *cl)
{
	m_client=static_cast<MapClient *>(cl);
}
