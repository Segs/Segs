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
		pktConnected *res = new pktConnected;
		m_proto->SendPacket(res);
		return true;
	}
	if(pak->m_opcode==9)
	{
		
		pktCS_SendEntity *in_pak = (pktCS_SendEntity *)pak;
		in_pak->dump();
		setClient(m_server->ClientExpected(getTargetAddr(),in_pak->m_cookie));
		//m_client->setHandler(this);
		ACE_ASSERT(m_client!=0);
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
		res->m_outdoor_map=0;
		res->m_map_number=1;
		res->m_map_desc="maps/City_Zones/City_00_01/City_00_01.txt";
		res->current_map_flags=1; //off 1
		res->m_trays.push_back("maps/City_Zones/City_00_01/City_00_01.txt");
		res->m_trays.push_back("object_library/Nature/SmPlants/SmPlants.txt");
		res->m_trays.push_back("object_library/Nature/Bushes/bushes/bushes.txt");
		res->m_trays.push_back("object_library/Nature/Bushes/HedgeRow/HedgeRow.txt");
		res->m_trays.push_back("object_library/Streets/elements/planters/planters.txt");
		res->m_trays.push_back("object_library/Nature/rocks/forest_rocks/forest_rocks.txt");
		res->m_trays.push_back("object_library/Nature/details/ground_cover/ground_cover.txt");
		res->m_trays.push_back("object_library/Nature/hills/hills.txt");
		res->m_trays.push_back("object_library/Nature/details/details.txt");
		res->m_trays.push_back("object_library/city_zones/twisted_trees/twisted_trees.txt");
		res->m_trays.push_back("object_library/Nature/rocks/nautre_rocks/nature_rocks.txt");
		res->m_trays.push_back("object_library/Nature/Trees/maple/maple.txt");
		res->m_trays.push_back("object_library/Omni/Omni.txt");
		res->m_trays.push_back("object_library/Omni/MissionBeacons/MissionBeacons.txt");
		res->m_trays.push_back("object_library/Nature/Trees/oval/oval.txt");
		res->m_trays.push_back("object_library/Nature/Elements/Ruined_Plant_Pots/Ruined_Plant_Pots.txt");
		res->m_trays.push_back("object_library/Buildings/skyline/skyline.txt");
		res->m_trays.push_back("object_library/Streets/elements/fire_escapes/fire_escapes.txt");
		res->m_trays.push_back("object_library/Streets_Ruined/Ruin_Elements/Ruin_Decals/Ruin_Decals.txt");
		res->m_trays.push_back("object_library/Common_Decor/Slums_&_Wastes/trash/trash.txt");
		res->m_trays.push_back("object_library/Streets/road_filler/road_filler.txt");
		res->m_trays.push_back("object_library/Streets_Ruined/Ruin_Elements/Ruin_fx/Ruin_fx.txt");
		res->m_trays.push_back("object_library/Streets/elements/Parked_Cars/Parked_Sport2/Parked_Sport2.txt");
		res->m_trays.push_back("object_library/Nature/grass_plates/grass_plates.txt");
		res->m_trays.push_back("object_library/Streets/road_filler/Parking_Lots/Parking_Lots.txt");
		res->m_trays.push_back("object_library/Streets/elements/walls/concrete1/concrete1.txt");
		res->m_trays.push_back("object_library/Streets/elements/Parked_Cars/Parked_Truck/Parked_Truck.txt");
		res->m_trays.push_back("object_library/Streets/elements/Parked_Cars/Parked_Compact/Parked_Compact.txt");
		res->m_trays.push_back("object_library/Streets/elements/Parked_Cars/Parked_Lux2/Parked_Lux2.txt");
		res->m_trays.push_back("object_library/Streets/elements/Parked_Cars/Parked_Sport1/Parked_Sport1.txt");
		res->m_trays.push_back("object_library/Streets/elements/misc_sm/misc_sm.txt");
		res->m_trays.push_back("object_library/Nature/sm_grass_plates/sm_grass_plates.txt");
		res->m_trays.push_back("object_library/Industrial/ind_grnd_plates/ind_grnd_plates.txt");
		res->m_trays.push_back("object_library/Streets/elements/signs/road_sm/road_sm.txt");
		res->m_trays.push_back("object_library/Collision/collision_walls/collision_walls.txt");
		res->m_trays.push_back("object_library/Common_Decor/Slums_&_Wastes/Junk_Sm_Items/Junk_Sm_Items.txt");
		res->m_trays.push_back("object_library/Buildings/Style/Filler_Shops/Filler_Shops.txt");
		res->m_trays.push_back("object_library/Doors/Store_Doors/Store_Doors.txt");
		res->m_trays.push_back("object_library/Doors/City_Doors/City_Doors.txt");
		res->m_trays.push_back("object_library/Common_Decor/WareHouse/ware_equipment/ware_equipment.txt");
		res->m_trays.push_back("object_library/Streets/road_2lane/road_2lane.txt");
		res->m_trays.push_back("object_library/Streets/elements/lights/streetlights/streetliths.txt");
		res->m_trays.push_back("object_library/Streets/road_crawlblock/road_crawlblock.txt");
		res->m_trays.push_back("object_library/Streets/elements/Parked_Cars/Parked_Lux1/Parked_Lux1.txt");
		res->m_trays.push_back("object_library/Buildings/Elements/roofs/ware_elmnts01/ware_elmnts01.txt");
		res->m_trays.push_back("object_library/city_zones/kings_row/kings_row_buildings/kings_row_buildings.txt");
		res->m_trays.push_back("object_library/Buildings/Elements/roofs/vents_etc/vents_etc.txt");
		res->m_trays.push_back("object_library/Buildings/Elements/bldg_lights/bldg_lights.txt");
		res->m_trays.push_back("object_library/walls/brick_parkwall/brick_parkwall.txt");
		res->m_trays.push_back("object_library/walls/walls_docks/walls_docks.txt");
		res->m_trays.push_back("object_library/Streets/elements/Graffiti/Graffiti.txt");
		res->m_trays.push_back("object_library/Industrial/ind_pipes/ind_pipes.txt");
		res->m_trays.push_back("object_library/walls/warzone_walls/warzone_walls.txt");
		res->m_trays.push_back("object_library/Streets/elements/Fence/barbwire/barbwire.txt");
		res->m_trays.push_back("object_library/Streets/elements/Fence/Chainlink/Chainlink.txt");
		res->m_trays.push_back("object_library/Streets/elements/gaurdrail/gaurdrail.txt");
		res->m_trays.push_back("object_library/Streets/elements/Fence/gate/gate.txt");
		res->m_trays.push_back("object_library/Doors/Wharehouse_Doors/Wharehouse_Doors.txt");
		res->m_trays.push_back("object_library/Streets/elements/Fence/slat/slat.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/5th_column/elements/lights/lights.txt");
		res->m_trays.push_back("object_library/Common_Decor/Slums_&_Wastes/Junk_Barrels/Junk_Barrels.txt");
		res->m_trays.push_back("object_library/WorldNPCS/Swat_Checkpoint/Swat_Checkpoint.txt");
		res->m_trays.push_back("object_library/Collision/no_teleport/no_teleport.txt");
		res->m_trays.push_back("object_library/walls/tunnels/tunnels.txt");
		res->m_trays.push_back("object_library/Streets/road_4lane/road_4lane.txt");
		res->m_trays.push_back("object_library/Streets/elements/lights/trafficlights/trafficlights.txt");
		res->m_trays.push_back("object_library/Streets/road_6lane/road_6lane.txt");
		res->m_trays.push_back("object_library/walls/citywalls/citywalls.txt");
		res->m_trays.push_back("object_library/Streets/road_filler/elevation_filler/elevation_filler.txt");
		res->m_trays.push_back("object_library/Streets/elements/marble_benches/marble_benches.txt");
		res->m_trays.push_back("object_library/city_zones/statues_clean/statues_clean_md/statues_clean_md.txt");
		res->m_trays.push_back("object_library/city_zones/canals/canal_plazas/canal_plazas.txt");
		res->m_trays.push_back("object_library/Streets/elements/fountains/fountains.txt");
		res->m_trays.push_back("object_library/Common_Decor/WareHouse/ware_CardBrd_Boxes/ware_CardBrd_Boxes.txt");
		res->m_trays.push_back("object_library/Buildings/Style/Filler_Buildings/Filler_Ruin1/Filler_Ruin1.txt");
		res->m_trays.push_back("object_library/Buildings/Elements/Doors/Dummy_Doors/Dummy_Doors.txt");
		res->m_trays.push_back("object_library/Doors/Residential_Doors/Residential_Doors.txt");
		res->m_trays.push_back("object_library/Buildings/Style/Filler_Buildings/Filler_Brick");
		res->m_trays.push_back("object_library/Streets/elements/Fence/woodfence/woodfence.txt");
		res->m_trays.push_back("object_library/Common_Decor/Common_Items/Lights/Exterior_Lights/Exterior_Lights.txt");
		res->m_trays.push_back("object_library/Buildings/Elements/signs/Special_Signs/Chaser_Sign_4to1/Chaser_Sign_4to1.txt");
		res->m_trays.push_back("object_library/Streets/overpass/overpass.txt");
		res->m_trays.push_back("object_library/Common_Decor/Slums_&_Wastes/Junk_Lg_Items/Junk_Lg_Items.txt");
		res->m_trays.push_back("object_library/Streets/elements/lights/gaslamps/gaslamps.txt");
		res->m_trays.push_back("object_library/Streets/elements/signs/Filler_Bldgs/Filler_Bldgs.txt");
		res->m_trays.push_back("object_library/Buildings/Style/deco/Deco17/Deco17.txt");
		res->m_trays.push_back("object_library/Buildings/Style/deco/Deco_elements/deco_boltons/deco_boltons.txt");
		res->m_trays.push_back("object_library/Buildings/Style/factories/fact_conveyors/fact_conveyors.txt");
		res->m_trays.push_back("object_library/Streets_Ruined/Ruin_road_filler/Ruin_road_filler.txt");
		res->m_trays.push_back("object_library/Common_Decor/Slums_&_Wastes/Ruin_Rail_Container/Ruin_Rail_Container.txt");
		res->m_trays.push_back("object_library/Streets_Ruined/Ruin_Elements/rubble/rubble.txt");
		res->m_trays.push_back("object_library/Nature/Trees/dead_tree/dead_tree.txt");
		res->m_trays.push_back("object_library/Streets_Ruined/Ruin_Elements/Ruined_Cars/Ruin_Compact/Ruin_Compact.txt");
		res->m_trays.push_back("object_library/Streets_Ruined/Ruin_Elements/Ruined_Cars/Ruin_Lux1/Ruin_Lux1.txt");
		res->m_trays.push_back("object_library/Streets_Ruined/Ruin_road_2lane/Ruin_road_2lane.txt");
		res->m_trays.push_back("object_library/Streets/elements/lights/ruined_streetlights/ruined_streetlights.txt");
		res->m_trays.push_back("object_library/Streets/elements/telepole/telepole.txt");
		res->m_trays.push_back("object_library/Streets_Ruined/Ruin_Elements/ruined_street_things/ruined_street_things.txt");
		res->m_trays.push_back("object_library/Streets_Ruined/Ruin_Elements/Ruined_Cars/Ruin_Truck/Ruin_Truck.txt");
		res->m_trays.push_back("object_library/Streets_Ruined/Ruin_Elements/Ruined_Cars/Ruin_Sport1/Ruin_Sport1.txt");
		res->m_trays.push_back("object_library/Streets_Ruined/Ruin_Elements/Ruined_Cars/Ruin_Sport2/Ruin_Sport2.txt");
		res->m_trays.push_back("object_library/Streets_Ruined/Ruin_Elements/Ruined_Cars/Ruin_Lux2/Ruin_Lux2.txt");
		res->m_trays.push_back("object_library/Streets_Ruined/eltrain_ruined/eltrain_ruined.txt");
		res->m_trays.push_back("object_library/Streets/eltrain/eltrain.txt");
		res->m_trays.push_back("object_library/Common_Decor/WareHouse/ware_crates_Med/ware_crates_Med.txt");
		res->m_trays.push_back("object_library/Common_Decor/WareHouse/cargo_cont/cargo_cont.txt");
		res->m_trays.push_back("object_library/Common_Decor/WareHouse/ware_crates_Sm/ware_crates_Sm.txt");
		res->m_trays.push_back("object_library/Common_Decor/WareHouse/ware_sundry/ware_sundry.txt");
		res->m_trays.push_back("object_library/Doors/Industrial_Doors/Industrial_Doors.txt");
		res->m_trays.push_back("object_library/Buildings/Elements/Decor_Exterior/Awning/awning.txt");
		res->m_trays.push_back("object_library/Buildings/Style/Ruin/Ruin_modules/Ruin_Modules.txt");
		res->m_trays.push_back("object_library/Streets_Ruined/Ruin_Elements/Ruin_Decals/Ruin_Decals.txt");
		res->m_trays.push_back("object_library/Buildings/Style/Ruin/Ruin_modules_LODS/Ruin_modules_LODS.txt");
		res->m_trays.push_back("object_library/Buildings/Elements/Ruined_Roofs/Ruined_Roofs.txt");
		res->m_trays.push_back("object_library/Buildings/Style/deco/Deco15/Deco15.txt");
		res->m_trays.push_back("object_library/Buildings/Style/factories/fact_stacks/fact_stacks.txt");
		res->m_trays.push_back("object_library/Buildings/Style/deco/Deco8/Deco8.txt");
		res->m_trays.push_back("object_library/Buildings/Style/deco/Deco4/Deco4.txt");
		res->m_trays.push_back("object_library/Buildings/Style/deco/Deco1/Deco1.txt");
		res->m_trays.push_back("object_library/Buildings/Style/deco/Deco13/Deco13.txt");
		res->m_trays.push_back("object_library/Buildings/Style/deco/Deco2/Deco2.txt");
		res->m_trays.push_back("object_library/Buildings/Style/deco/Deco11/Deco11.txt");
		res->m_trays.push_back("object_library/Buildings/Style/deco/Deco12/Deco12.txt");
		res->m_trays.push_back("object_library/Buildings/Style/deco/Deco10/Deco10.txt");
		res->m_trays.push_back("object_library/Buildings/Style/deco/Deco16/Deco16.txt");
		res->m_trays.push_back("object_library/Buildings/Style/deco/Deco9/Deco9.txt");
		res->m_trays.push_back("object_library/Buildings/Style/deco/Deco5/Deco5.txt");
		res->m_trays.push_back("object_library/Special_Buildings/Hospital/Hospital.txt");
		res->m_trays.push_back("object_library/Streets/elements/signs/xtralarge/xtralarge.txt");
		res->m_trays.push_back("object_library/Common_Decor/Med_&_Sci/Medical_Items/Medical_Items.txt");
		res->m_trays.push_back("object_library/Common_Decor/Office/Office_Tech/Office_Tech.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/office01/elements/details/detais.txt");
		res->m_trays.push_back("object_library/Special_Buildings/Medical_Clinic/Medical_Clinic.txt");
		res->m_trays.push_back("object_library/Buildings/Elements/Doors/Door_Shop01/Door_Shop01.txt");
		res->m_trays.push_back("object_library/Common_Decor/Common_Items/Lights/Ceiling_Lights/Ceiling_Lights.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/office01/elements/whiteboards/whiteboards.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/office01/elements/misc/misc.txt");
		res->m_trays.push_back("object_library/Common_Decor/Bar/Bar.txt");
		res->m_trays.push_back("object_library/Item_library/Healthkit/Healthkit.txt");
		res->m_trays.push_back("object_library/Common_Decor/Common_Items/Lights/Lamps/Lamps.txt");
		res->m_trays.push_back("object_library/Common_Decor/Common_Items/Chairs/Chairs.txt");
		res->m_trays.push_back("object_library/Common_Decor/Common_Items/Desks_&_Tables/Desks_&_Tables.txt");
		res->m_trays.push_back("object_library/Common_Decor/Common_Items/Plants/Plants.txt");
		res->m_trays.push_back("object_library/Common_Decor/Office/Office_Utility/Office_Utility.txt");
		res->m_trays.push_back("object_library/Common_Decor/Med_&_Sci/Reclamation_Pad/Reclamation_Pad.txt");
		res->m_trays.push_back("object_library/Buildings/Style/deco/Deco_elements/deco_fixtures/deco_fixtures.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/office01/trays/halls/halls.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/office01/structure/misc/misc.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/office01/elements/lights/lights.txt");
		res->m_trays.push_back("object_library/Omni/EncounterSpawns/EncounterSpawns.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/office01/elements/furniture/furniture.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/office01/structure/tilefloors/tilefloors.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/office01/elements/art/art.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/office01/structure/walls/walls.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/office01/elements/counters/counters.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/office01/elements/trees/trees.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/office01/elements/planters/planters.txt");
		res->m_trays.push_back("object_library/Special_Buildings/phalanx/phlx_interior/phlx_interior.txt");
		res->m_trays.push_back("object_library/Omni/ElevatorPads/ElevatorPads.txt");
		res->m_trays.push_back("object_library/Item_library/Mission_Items/Mission_Items.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/cot/elements/magicrunes/magicrunes.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/cot/elements/small_detail/small_detail.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/tech/elements/machines/machines.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/5th_column/elements/computers/computers.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/5th_column/elements/crates_barrels/crates_barrels.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/tech/elements/computers/computers.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/tech/elements/smalldetail/smalldetail.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/Caves/elements/rocks/rocks.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/5th_column/elements/misc_decor/misc_decor.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/trays/trial_room/trial_room.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/elements/misc/misc.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/elements/pipes/pipes.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/elements/computers/computers.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/elements/Cranks_Levers/Cranks_Levers.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/elements/box_structure/box_structure.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/structure/catwalks/catwalks.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/trays/Rooms/room_water/room_water.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/structure/sidewalks/sidewalks.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/structure/walls/walls.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/elements/Boiler_Makers/Boiler_Makers.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/structure/exit/exit.txt");
		res->m_trays.push_back("object_library/Doors/Sewer_Doors/Sewer_Doors.txt");
		res->m_trays.push_back("object_library/city_templates/mapmarkers/mapmarkers.txt");
		res->m_trays.push_back("object_library/Buildings/Style/warehouses/ind_elements_01/ind_elements_01.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/elements/lights/lights.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/structure/platforms/platforms.txt");
		res->m_trays.push_back("object_library/Streets/elements/Docks/dock_lights/dock_lights.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/elements/Churns/Churns.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/elements/signs/signs.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/elements/cyl_structure/cyl_structure.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/elements/CHUNKS/CHUNKS.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/sewers/structure/rock_structure/rock_structure.txt");
		res->m_trays.push_back("object_library/Omni/EnvironmentCollision/EnvironmentCollision.txt");
		res->m_trays.push_back("object_library/Item_library/objective_groups/objective_groups.txt");
		res->m_trays.push_back("object_library/city_zones/hollows/hollows_elements/hollows_obelisks/hollows_obelisks.txt");
		res->m_trays.push_back("object_library/Special_Buildings/Reactor_control/Rctr_elemts/Rctr_elemts.txt");
		res->m_trays.push_back("object_library/city_zones/shadow_shard/ss_monuments/ss_monuments.txt");
		res->m_trays.push_back("object_library/city_zones/shadow_shard/ss_kora_fruit/ss_kora_fruit.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/unique/Devouring_office/DE_Wall_decals/DE_Wall_decals.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/unique/Mek_Man_Factory/Mek_Man_Parts/Mek_Man_Parts.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/unique/Mek_Man_Factory/MM_Machines/mm_machines.txt");
		res->m_trays.push_back("object_library/Industrial/caps_n_wires/caps_n_wires.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/5th_column/elements/pipes/pipes.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/5th_column/elements/machines_etc/machines_etc.txt");
		res->m_trays.push_back("object_library/city_zones/plaques_dirty/plaques_dirty.txt");
		res->m_trays.push_back("object_library/Common_Decor/Common_Items/Tv_&_Electronic/Tv_&_Electronic.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/cot/elements/crystals/crystals.txt");
		res->m_trays.push_back("object_library/Villain_Lairs/cot/elements/big_detail/big_detail.txt");
		res->m_trays.push_back("object_library/Streets_Ruined/Ruin_Elements/Ruin_Signs/ruin_xtralarge/ruin_xtralarge.txt");
		//res->m_trays.push_back("object_library/Omni/EncounterSpawns/City_00_01/City_00_01.txt");
		res->m_trays.push_back("object_library/city_templates/map_2d_blackpoly/map_2d_blackpoly.txt");
		res->m_trays.push_back("object_library/Omni/EncounterSpawns/BaseTypes/BaseTypes.txt");
		res->m_trays.push_back("object_library/Omni/EncounterSpawns/City_01_03/City_01_03.txt");
		res->m_trays.push_back("object_library/Omni/neighborhood_markers/neighborhood_markers.txt");
		res->m_trays.push_back("object_library/Streets/elements/information_post/information_post.txt");
		res->m_trays.push_back("object_library/Omni/City_Volumes/City_Volumes.txt");
		res->num_base_elems=res->m_trays.size();
		res->m_crc.resize(res->num_base_elems);
		Matrix4x3 mat;
		for(size_t j=0; j<sizeof(Matrix4x3)/4; j++)
		{
			((float *)&mat.row1)[j]=0.0f;
		}
		mat.row1.vals.x = mat.row2.vals.y = mat.row3.vals.z= 1.0;

		res->m_refs.push_back(MapRef(0,mat,"grp_Geometry"));
		res->m_refs.push_back(MapRef(1,mat,"grp_spawndefs"));
		res->m_refs.push_back(MapRef(2,mat,"grp_blackmap"));
		res->m_refs.push_back(MapRef(3,mat,"grp_PersistentNPC"));
		res->m_refs.push_back(MapRef(4,mat,"grp_Atmosphere_NPCs"));
		res->m_refs.push_back(MapRef(5,mat,"grp_MissionMapSwap"));
		res->m_refs.push_back(MapRef(6,mat,"grp_neighborhoods"));
		res->m_refs.push_back(MapRef(7,mat,"grp_beacons"));
		res->m_refs.push_back(MapRef(8,mat,"grp_audio"));
		res->m_refs.push_back(MapRef(9,mat,"grp_Plaques"));
		res->m_refs.push_back(MapRef(10,mat,"grp_warpvolumes"));

		res->ref_crc=0;
		res->ref_count=11;
		res->unkn1=1;
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("%d - %d - %d\n"),res->unkn1,
			res->undos_PP,
			res->current_map_flags));
		res->unkn2=1;
		m_proto->SendPacket(res);
		return true;
	}
	if(pak->m_opcode==5)
	{
		pktMap_Server_EntitiesResp * res = new pktMap_Server_EntitiesResp(m_client);
		//pktMap_Server_Connect* res = new pktMap_Server_Connect;
		Entity *pent = m_client->getCharEntity(); //new Entity;//EntityManager::CreatePlayer();
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
		pent->m_costume.m_costume_type=2; // npc costume for now
		
		m_client->getCurrentMap()->m_entities.m_entlist.push_back(pent);

		pent = new Entity;
		pent->m_idx=1;
		pent->m_create=true;
		pent->var_C=true;
		pent->var_129C=true; // will break entity-receiving loop
		m_client->getCurrentMap()->m_entities.m_entlist.push_back(pent);

		res->entReceiveUpdate=false;
//		res->m_resp=1;
		res->unkn1=false;
		res->m_num_commands=0;
		res->abs_time = (u32)time(NULL);
		res->unkn2=true; // default parameters for first flags

		Entity door1;
		door1.m_idx=0x68;
		door1.m_create=true;
		door1.var_129C=false;
		door1.field_64=0x6A;
		door1.m_type=8;
		door1.m_hasname=true;
		door1.m_name="Dr";
		door1.field_60=0x00003D96;
		door1.m_hasgroup_name=false;
		door1.var_C=true;
		door1.m_rare_bits=true;
		door1.m_state_mode_send=true;
		door1.m_state_mode=0;
		door1.pos = Vector3(685,-95.9531,539.016);
												//0x100,0xE,
		door1.qrot= Quaternion().FromNormalized(0.0,-0.996312,0.0);
		door1.m_seq_update=true;
		door1.m_seq_upd_num1=0;
		door1.m_seq_upd_num2=0;
		door1.m_pchar_things=0;
		door1.m_costume.m_costume_type=2;
		door1.m_costume.costume_type_idx_P=0x356;
		door1.m_costume.costume_sub_idx_P =0;
		//xluency=1
		door1.m_odd_send=false;
		door1.m_is_villian=false;
		door1.m_contact=false;
		door1.entReceiveAlwaysCon=false;
		door1.entReceiveSeeThroughWalls=false;
		door1.m_SG_info=true;
		door1.field_78=0;

		Entity door2;
		door2.m_idx=0x69;
		door2.m_create=true;
		door2.var_129C=false;
		door2.field_64=0x6B;
		door2.m_type=8;
		door2.m_hasname=true;
		door2.m_name="Dr";
		door2.field_60=0x0000622D;
		door2.m_hasgroup_name=false;
		door2.var_C=true;
		door2.m_rare_bits=true;
		door2.m_state_mode_send=true;
		door2.m_state_mode=0;
		door2.pos = Vector3(195.52,-96.0,591.672);
											//0x100,0x7F,0x100
		door2.qrot= Quaternion().FromNormalized(0.0,-0.711432,0.0);
		door2.m_seq_update=true;
		door2.m_seq_upd_num1=0;
		door2.m_seq_upd_num2=0;
		door2.m_pchar_things=0;
		door2.m_costume.m_costume_type=2;
		door2.m_costume.costume_type_idx_P=0x356;
		door2.m_costume.costume_sub_idx_P =0;
		//door2.m_ragdol_num_bones=0;
		//xluency=1
		door2.m_odd_send=false;
		door2.m_is_villian=false;
		door2.m_contact=false;
		door2.entReceiveAlwaysCon=false;
		door2.entReceiveSeeThroughWalls=false;
		door2.m_SG_info=true;
		door2.field_78=0;
		Entity m_player1;
		m_player1.m_idx=0x6A;
		m_player1.m_create=true;
		m_player1.var_129C=false;
		m_player1.field_60=0x579C;
		m_player1.field_68=0x17D8;
		m_player1.m_type=2;
		m_player1.m_create_player=0;
		m_player1.m_player_villain=0;
		m_player1.m_origin_idx=0;
		m_player1.m_class_idx=3;
		m_player1.m_selector1=1;
		m_player1.m_num_titles=0;
		m_player1.m_hasname=true;
		m_player1.m_name="Jubal Early";
		m_player1.field_60=0x00001AC3;
		m_player1.var_C=true;
		m_player1.m_rare_bits=true;
		m_player1.m_state_mode_send=true;
		m_player1.m_state_mode=0;
		m_player1.pos = Vector3(-65.0625,0.0,189.531);
		//0x100,0x1F0,0x100
		m_player1.qrot= Quaternion().FromNormalized(0.0,0.99518,0.0);
		m_player1.m_seq_update=true;
		m_player1.m_seq_upd_num1=0x2A;
		m_player1.m_seq_upd_num2=0;
		m_player1.m_pchar_things=1;
		m_player1.m_num_fx=1;
		m_player1.m_fx1.push_back(2);
		m_player1.m_fx2.push_back(0xAE4C);

		m_player1.m_costume.m_costume_type=2;
		m_player1.m_costume.costume_type_idx_P=0x356;
		m_player1.m_costume.costume_sub_idx_P =0;
		//door2.m_ragdol_num_bones=0;
		//xluency=1
		m_player1.m_odd_send=false;
		m_player1.m_is_villian=false;
		m_player1.m_contact=false;
		m_player1.entReceiveAlwaysCon=false;
		m_player1.entReceiveSeeThroughWalls=false;
		m_player1.m_SG_info=true;
		m_player1.field_78=0;
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
	if(pak->m_opcode==6)
	{
		pak->dump();
		m_proto->SendPacket(new pktIdle);
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