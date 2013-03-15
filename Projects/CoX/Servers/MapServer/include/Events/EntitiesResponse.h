#pragma once
#include <vector>
#include <string>
#include "BitStream.h"
#include "MapRef.h"
#include "CRUD_Events.h"
#include "MapLink.h"

class MapClient;
typedef CRUDLink_Event MapLinkEvent; //<MapLink>

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
        static int hour = 10;
        tgt.StorePackedBits(1,1); // use 'time' shortcut
        tgt.StoreFloat((hour++)/10.0f);
        tgt.StorePackedBits(1,2); // use 'time scale' shortcut
        tgt.StoreFloat(8.0f);
        tgt.StorePackedBits(1,3); // use 'time step scale' shortcut
        tgt.StoreFloat(4.0f);
        tgt.StorePackedBits(1,m_num_commands);
    }
    virtual void serializeto(BitStream &tgt) const;

    bool entReceiveUpdate;
    bool unkn1;
    bool unkn2;
    bool debug_info;
    bool m_interpolating;
    uint32_t abs_time;
    uint32_t db_time;
    uint16_t m_debug_idx;
    uint8_t m_interpolation_level;
    uint8_t m_interpolation_bits;
    uint32_t m_num_commands;
    uint32_t m_command_idx[15];
    std::string m_commands[15];
    uint32_t m_num_commands2;
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
