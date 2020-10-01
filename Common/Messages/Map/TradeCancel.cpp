/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "TradeCancel.h"
#include "MapEvents.h"

namespace SEGSEvents
{


TradeCancel::TradeCancel()
    : GameCommandEvent(MapEventTypes::evTradeCancel)
{
}

TradeCancel::TradeCancel(const QString& msg)
    : GameCommandEvent(MapEventTypes::evTradeCancel)
    , m_msg(msg)
{
}

void TradeCancel::serializeto(BitStream& bs) const {
    bs.StorePackedBits(1, type() - MapEventTypes::evFirstServerToClient);
    bs.StoreString(m_msg);
}


} // namespace SEGSEvents
//! @}
