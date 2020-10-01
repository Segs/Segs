/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "TradeUpdate.h"

#include "MapEvents.h"
#include "GameData/Powers.h"

namespace SEGSEvents
{


namespace {

static std::vector<CharacterEnhancement> getTradedEnhancements(const Entity& ent, const TradeInfo& info)
{
    std::vector<CharacterEnhancement> result;

    for (const uint32_t idx : info.m_enhancements)
    {
       const CharacterEnhancement* enh = getEnhancement(ent, idx);
       if(enh != nullptr)
       {
           result.push_back(*enh);
       }
    }

    return result;
}

static std::vector<CharacterInspiration> getTradedInspirations(const Entity& ent, const TradeInfo& info)
{
    std::vector<CharacterInspiration> result;

    for (const TradeInspiration& trade_insp : info.m_inspirations)
    {
        const CharacterInspiration* insp = getInspiration(ent, trade_insp.m_col, trade_insp.m_row);
        if(insp != nullptr)
        {
            result.push_back(*insp);
        }
    }

    return result;
}

static void serializePowerInfo(BitStream& bs, const PowerPool_Info& info)
{
    bs.StorePackedBits(3, info.m_pcat_idx);
    bs.StorePackedBits(3, info.m_pset_idx);
    bs.StorePackedBits(3, info.m_pow_idx);
}

static void serializeEnhancement(BitStream& bs, const CharacterEnhancement& enh)
{
    serializePowerInfo(bs, enh.m_enhance_info);
    bs.StorePackedBits(5, enh.m_level);
    bs.StorePackedBits(2, enh.m_num_combines);
}

static void serializeInspiration(BitStream& bs, const CharacterInspiration& insp)
{
    serializePowerInfo(bs, insp.m_insp_info);
}

} // anonymous namespace


TradeUpdate::TradeUpdate()
    : GameCommandEvent(MapEventTypes::evTradeUpdate)
{
}

TradeUpdate::TradeUpdate(const TradeMember& trade_self, const TradeMember& trade_other, const Entity& entity_other)
    : GameCommandEvent(MapEventTypes::evTradeUpdate)
    , m_trade_self(trade_self)
    , m_trade_other(trade_other)
{
    m_enhancements = getTradedEnhancements(entity_other, trade_other.m_info);
    m_inspirations = getTradedInspirations(entity_other, trade_other.m_info);
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

    for (const CharacterEnhancement& enh : m_enhancements)
    {
       serializeEnhancement(bs, enh);
    }

    for (const CharacterInspiration& insp : m_inspirations)
    {
       serializeInspiration(bs, insp);
    }
}


} // namespace SEGSEvents
//! @}
