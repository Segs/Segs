/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "TradeSuccess.h"
#include "MapEvents.h"


TradeSuccess::TradeSuccess(const QString& msg)
    : GameCommand(MapEventTypes::evTradeSuccess)
    , m_msg(msg)
{
}

void TradeSuccess::serializeto(BitStream& bs) const {
    bs.StorePackedBits(1, type() - MapEventTypes::evFirstServerToClient);
    bs.StoreString(m_msg);
}

//! @}
