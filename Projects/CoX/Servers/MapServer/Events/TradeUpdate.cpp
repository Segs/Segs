/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "TradeUpdate.h"

#include "MapEvents.h"
#include "NetStructures/Powers.h"

namespace  {

void serializePowerInfo(BitStream& bs, const PowerPool_Info& info)
{
    bs.StorePackedBits(3, info.m_pcat_idx);
    bs.StorePackedBits(3, info.m_pset_idx);
    bs.StorePackedBits(3, info.m_pow_idx);
}

void serializeEnhancement(BitStream& bs, const CharacterEnhancement& enh)
{
    serializePowerInfo(bs, enh.m_enhance_info);
    bs.StorePackedBits(5, enh.m_level);
    bs.StorePackedBits(2, enh.m_num_combines);
}

void serializeInspiration(BitStream& bs, const CharacterInspiration& insp)
{
    serializePowerInfo(bs, insp.m_insp_info);
}

} // anonymous namespace


TradeUpdate::TradeUpdate(const TradeMember& trade_self, const TradeMember& trade_other, const Entity& entity_other)
    : GameCommand(MapEventTypes::evTradeUpdate)
    , m_trade_self(trade_self)
    , m_trade_other(trade_other)
    , m_entity_other(entity_other)
{
}

void TradeUpdate::serializeto(BitStream& bs) const {
    const TradeInfo& info_self = m_trade_self.m_info;
    const TradeInfo& info_other = m_trade_other.m_info;

    bs.StorePackedBits(1, type() - MapEventTypes::evFirstServerToClient);
    bs.StorePackedBits(1, m_trade_other.m_db_id);

    bs.StorePackedBits(1, info_self.m_accepted);
    bs.StorePackedBits(1, info_other.m_accepted);

    bs.StorePackedBits(1, info_self.m_influence);
    bs.StorePackedBits(1, info_other.m_influence);

    bs.StorePackedBits(1, info_other.m_enhancements.size());
    bs.StorePackedBits(1, info_other.m_inspirations.size());

    for (const uint32_t idx : info_other.m_enhancements)
    {
       const CharacterEnhancement* enh = getEnhancement(m_entity_other, idx);
       if (enh != nullptr)
       {
           serializeEnhancement(bs, *enh);
       }
       else
       {
           // Write dummy values to uphold the communication protocol.
           serializeEnhancement(bs, CharacterEnhancement());
       }
    }

    for (const TradeInspiration& trade_insp : info_other.m_inspirations)
    {
        const CharacterInspiration* insp = getInspiration(m_entity_other, trade_insp.m_col, trade_insp.m_row);
        if (insp != nullptr)
        {
            serializeInspiration(bs, *insp);
        }
        else
        {
            // Write dummy values to uphold the communication protocol.
            serializeInspiration(bs, CharacterInspiration());
        }
    }
}

//! @}
