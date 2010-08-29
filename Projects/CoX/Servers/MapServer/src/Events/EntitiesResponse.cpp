#include <ace/Assert.h>
#include "Entity.h"
#include "Events/EntitiesResponse.h"
#include "MapEvents.h"
#include "MapClient.h"
#include "MapHandler.h"
#include "MapInstance.h"
//! EntitiesResponse is sent to a client to inform it about the current world state.
EntitiesResponse::EntitiesResponse(MapClient *cl, bool inc) :
    MapLinkEvent(MapEventTypes::evEntitites),
    m_incremental(inc)
{
    m_num_commands = 0;
    m_client = cl;
    abs_time=db_time=0;
    unkn2=true;

}
void EntitiesResponse::serializeto( BitStream &tgt ) const
{
    EntityManager &ent_manager=m_client->current_map()->m_entities;
    tgt.StorePackedBits(1,m_incremental ? 2 : 3); // opcode
    tgt.StoreBits(1,entReceiveUpdate);
    sendCommands(tgt);

    tgt.StoreBits(32,abs_time);
    //tgt.StoreBits(32,db_time);

    tgt.StoreBits(1,unkn2);
    if(unkn2==0)
    {
        tgt.StoreBits(1,debug_info);
        tgt.StoreBits(1,selector1);
        if(selector1==1)
        {
            tgt.StoreBits(2,dword_A655C0);
            tgt.StoreBits(2,BinTrees_PPP);
        }
    }

    //else debug_info = false;
    ent_manager.sendEntities(tgt);
    if(debug_info&&!unkn2)
    {
        ent_manager.sendDebuggedEntities(tgt);
        ent_manager.sendGlobalEntDebugInfo(tgt);
    }
    sendServerPhysicsPositions(tgt); // These are not client specific ?
    sendControlState(tgt);// These are not client specific ?
    ent_manager.sendDeletes(tgt);
    // Client specific part
    sendClientData(tgt);
}
void EntitiesResponse::sendClientData(BitStream &tgt) const
{
    PlayerEntity *ent=static_cast<PlayerEntity *>(m_client->char_entity());
    Character &player_char=ent->m_char;
    if(!m_incremental)
    {
        // initial character update = level/name/class/origin/map_name
        //m_client->char_entity()->m_char.m_ent=m_client->char_entity();
        ent->serialize_full(tgt);
        player_char.sendTray(tgt);
        player_char.sendTrayMode(tgt);
        tgt.StoreString(ent->name()); // maxlength 32

        tgt.StoreString(ent->m_battle_cry); //max 128
        tgt.StoreString(ent->m_character_description); //max 1024
        player_char.sendWindows(tgt);
        tgt.StoreBits(1,0); // lfg related
        tgt.StoreBits(1,0); // pEnt->player_ppp.field_984C
        player_char.sendTeamBuffMode(tgt);
        player_char.sendDockMode(tgt);
        player_char.sendChatSettings(tgt);
        player_char.sendTitles(tgt);
        player_char.sendDescription(tgt);
        u8 auth_data[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        tgt.StoreBitArray(auth_data,128);
        player_char.sendKeybinds(tgt);
        player_char.sendOptions(tgt);
        player_char.sendFriendList(tgt);
    }
    else
    {
        ent->m_char.sendFullStats(tgt);
    }
    storePowerInfoUpdate(tgt);
    //storePowerModeUpdate(tgt);
    //storeBadgeUpdate(tgt);
    //storeGenericinventoryUpdate(tgt);
    //storeInventionUpdate(tgt);
    storeTeamList(tgt);
    storeSuperStats(tgt);
    storeGroupDyn(tgt);
    bool additional=false;
    tgt.StoreBits(1,additional);
    if(additional)
    {
        tgt.StoreFloat(0.1f);
        tgt.StoreFloat(0.2f); // camera_yaw
        tgt.StoreFloat(0.3f);
    }
}
void EntitiesResponse::sendControlState(BitStream &bs) const
{
    sendServerControlState(bs);
}
void EntitiesResponse::sendServerPhysicsPositions(BitStream &bs) const
{
    bs.StoreBits(1,0);
    bs.StoreBits(1,0);
}
void EntitiesResponse::sendServerControlState(BitStream &bs) const
{
    bs.StoreBits(1,0);
    bs.StoreBits(1,0);
}
void EntitiesResponse::storePowerInfoUpdate(BitStream &bs) const
{
    bool powerinfo_available=false;
    bs.StoreBits(1,powerinfo_available);
    if ( powerinfo_available )
    {
        bs.StoreBits(1,false);
        bs.StorePackedBits(1,0);
        // send powers
        bs.StoreBits(1,0);

        bs.StorePackedBits(4,0);
        bs.StorePackedBits(1,0);
        // ... missing
    }
    // send powers
    bs.StoreBits(1,0);
}
void EntitiesResponse::storePowerModeUpdate(BitStream &bs) const
{
    bs.StoreBits(1,0);
    if(false)
    {
        bs.StorePackedBits(3,1);
        for(int i=0; i<1; i++)
        {
            bs.StorePackedBits(3,0);
        }
    }
}
void EntitiesResponse::storeBadgeUpdate(BitStream &bs) const
{
    bs.StoreBits(1,0);
    bs.StoreBits(1,0);
}
void EntitiesResponse::storeGenericinventoryUpdate(BitStream &bs)const
{
    bs.StorePackedBits(1,0);
}
void EntitiesResponse::storeInventionUpdate(BitStream &bs)const
{
    bs.StorePackedBits(1,0);
}
void EntitiesResponse::storeTeamList(BitStream &bs) const
{
    int team_id=0; // 
    NetStructure::storePackedBitsConditional(bs,20,team_id);
    //storePackedBitsConditional(bs,20,0);
    bs.StoreBits(1,0);
    bs.StoreBits(1,0);
    if(team_id>0)
    {
        bs.StoreBits(32,0);
        int num_members=0;
        bs.StorePackedBits(1,num_members);
    }
}
void EntitiesResponse::storeSuperStats(BitStream &bs) const
{
    bs.StorePackedBits(1,0);
}
void EntitiesResponse::storeGroupDyn(BitStream &bs) const
{
    bs.StorePackedBits(1,0);
}

