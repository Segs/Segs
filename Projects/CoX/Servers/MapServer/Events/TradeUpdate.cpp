/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "TradeUpdate.h"

#include "MapEvents.h"
#include "NetStructures/Trade.h"


TradeUpdate::TradeUpdate(const TradeMember& trade_self, const TradeMember& trade_other)
    : GameCommand(MapEventTypes::evTradeUpdate)
    , m_db_id(trade_other.m_db_id)
    , m_self_accepted(trade_self.m_accepted)
    , m_other_accepted(trade_other.m_accepted)
    , m_self_influence(trade_self.m_influence)
    , m_other_influence(trade_other.m_influence)
{
    // TODO: Inspirations and enhancements.
}

void TradeUpdate::serializeto(BitStream& bs) const {
    bs.StorePackedBits(1, type() - MapEventTypes::evFirstServerToClient);
    bs.StorePackedBits(1, m_db_id);

    bs.StorePackedBits(1, m_self_accepted);
    bs.StorePackedBits(1, m_other_accepted);

    bs.StorePackedBits(1, m_self_influence);
    bs.StorePackedBits(1, m_other_influence);

    bs.StorePackedBits(1, 0); // TODO: Vector size.
    bs.StorePackedBits(1, 0); // TODO: Vector size.

    // TODO: Inspirations and enhancements.
}

// FROM CLIENT CODE: TODO: Delete when serialization is properly implemented.
//   my_pakGetPackedBits(pak, 1);
//   g_trade.aAccepted = my_pakGetPackedBits(pak, 1);
//   g_trade.bAccepted = my_pakGetPackedBits(pak, 1);
//   g_trade.aInfluence = my_pakGetPackedBits(pak, 1);
//   g_trade.bInfluence = my_pakGetPackedBits(pak, 1);
//   g_trade.bNumSpec = my_pakGetPackedBits(pak, 1);
//   g_trade.bNumInsp = my_pakGetPackedBits(pak, 1);
//   trade_clear(1);
//   for ( i = 0; i < g_trade.bNumSpec; ++i )
//   {
//       g_trade.bSpec[i].m_template = entReceivePower(pak, &g_ppPowerSets);
//       g_trade.bSpec[i].iLevel = my_pakGetPackedBits(pak, 5);
//       g_trade.bSpec[i].iNumCombines = my_pakGetPackedBits(pak, 2);
//   }
//   for ( j = 0; j <g_trade.bNumInsp; ++j )
//   {
//       g_trade.bInsp[j] = entReceivePower(pak, &g_ppPowerSets);
//   }

//    Power_Data *entReceivePower(DbPacket *pak, PowerSet_Glob *pdict)
//    {
//        PowerTripartId pid;
//        pid.category_idx = my_pakGetPackedBits(pak, 3);
//        pid.powerset_entry_idx = my_pakGetPackedBits(pak, 3);
//        pid.power_idx = my_pakGetPackedBits(pak, 3);
//        Power_Data *pdat = get_base_power_from3ints(pdict, &pid);
//        if ( !pdat )
//            coh_printf("BAD BAD: Bad power received (ignore and player were be kicked)\n");
//        return pdat;
//    }

//! @}
