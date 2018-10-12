/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "TradeSuccess.h"
#include "MapEvents.h"


namespace SEGSEvents
{


TradeSuccess::TradeSuccess()
    : GameCommandEvent(MapEventTypes::evTradeSuccess)
{
}

TradeSuccess::TradeSuccess(const QString& msg)
    : GameCommandEvent(MapEventTypes::evTradeSuccess)
    , m_msg(msg)
{
}

void TradeSuccess::serializeto(BitStream& bs) const {
    bs.StorePackedBits(1, type() - MapEventTypes::evFirstServerToClient);
    bs.StoreString(m_msg);
}


} // namespace SEGSEvents
//! @}
