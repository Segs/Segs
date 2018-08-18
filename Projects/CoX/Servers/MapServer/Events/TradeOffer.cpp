/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "TradeOffer.h"
#include "MapEvents.h"


TradeOffer::TradeOffer(uint32_t db_id, const QString& name)
    : GameCommand(MapEventTypes::evTradeOffer)
    , m_db_id(db_id)
    , m_name(name)
{
}

void TradeOffer::serializeto(BitStream& bs) const {
    bs.StorePackedBits(1, type() - MapEventTypes::evFirstServerToClient);
    bs.StoreBits(32, m_db_id);
    bs.StoreString(m_name);
}

//! @}
