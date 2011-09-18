#pragma once
#include <vector>
#include <string>
#include "BitStream.h"
#include "MapRef.h"
#include "CRUD_Events.h"
#include "MapLink.h"

class MapClient;
typedef CRUDLink_Event<MapLink> MapLinkEvent;

class EntitiesResponse : public MapLinkEvent
{
protected:
    MapClient *m_client;
    bool m_incremental; //  if true then this is incremental update
public:
    EntitiesResponse(MapClient *cl);
    void is_incremental(bool v) {m_incremental=v;}
    virtual void serializefrom(BitStream &)
    {
    }
    void sendCommands(BitStream &tgt) const
    {
        tgt.StorePackedBits(1,m_num_commands);
    }
    virtual void serializeto(BitStream &tgt) const;

    bool entReceiveUpdate;
    bool unkn1;
    bool unkn2;
    bool debug_info;
    bool m_interpolating;
    u32 abs_time;
    u32 db_time;
    u32 u1;
    u32 u2;
    u32 u3;
    u16 m_debug_idx;
    u8 m_interpolation_level;
    u8 m_interpolation_bits;
    u32 m_num_commands;
    u32 m_command_idx[15];
    std::string m_commands[15];
    u32 m_num_commands2;
    std::string m_commands2[15];
private:
    void sendClientData(BitStream &bs) const;
    void sendServerPhysicsPositions(BitStream &bs) const;
    void sendControlState(BitStream &bs) const;
    void sendServerControlState(BitStream &bs) const;
    void storePowerInfoUpdate(BitStream &bs) const;
    void storePowerModeUpdate(BitStream &bs) const;
    void storeBadgeUpdate(BitStream &bs) const;
    void storeGenericinventoryUpdate(BitStream &bs) const;
    void storeInventionUpdate(BitStream &bs) const;
    void storeTeamList(BitStream &bs) const;
    void storeSuperStats(BitStream &bs) const;
    void storeGroupDyn(BitStream &bs) const;
};
