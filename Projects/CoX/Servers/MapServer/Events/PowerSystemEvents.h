/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommandList.h"

#include "MapEvents.h"
#include "MapLink.h"

#include <QtCore/QString>
namespace SEGSEvents
{
// [[ev_def:type]]
class CombineEnhanceResponse final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    uint32_t            m_was_success;
    // [[ev_def:field]]
    uint32_t            m_destroyed_on_fail;
    explicit    CombineEnhanceResponse() : GameCommandEvent(evCombineEnhanceResponse) {}
                CombineEnhanceResponse(uint32_t success, uint32_t destroy) : GameCommandEvent(evCombineEnhanceResponse),
                    m_was_success(success),
                    m_destroyed_on_fail(destroy)
                {
                }

        void    serializeto(BitStream &bs) const override {
                    bs.StorePackedBits(1, type()-evFirstServerToClient); // 56

                    bs.StorePackedBits(1, m_was_success);
                    bs.StorePackedBits(1, m_destroyed_on_fail);
                }
        EVENT_IMPL(CombineEnhanceResponse)

};

// [[ev_def:type]]
class CombineEnhancementsReq final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    EnhancemenSlotEntry first_power;
    // [[ev_def:field]]
    EnhancemenSlotEntry second_power;
    CombineEnhancementsReq() : MapLinkEvent(evCombineEnhancementsReq)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1,40); // opcode
        assert(false); // since we will not send CombineEnhancements to anyone :)
    }
    // now to make this useful ;)

    void getPowerForCombine(BitStream &bs, EnhancemenSlotEntry &entry)
    {
        // first bit tells us if we have full/partial?? data
        // here we can do a small refactoring, because in both branches of if/else, the last set value is index.
        entry.m_set_in_power = bs.GetBits(1);
        if(entry.m_set_in_power)
        {
            entry.m_pset_idx = bs.GetPackedBits(1);
            entry.m_pow_idx = bs.GetPackedBits(1);
        }
        entry.m_eh_idx = bs.GetPackedBits(1);
    }
    void serializefrom(BitStream &bs) override
    {
        getPowerForCombine(bs, first_power);
        getPowerForCombine(bs, second_power);
    }
    EVENT_IMPL(CombineEnhancementsReq)

};

// [[ev_def:type]]
class MoveEnhancement final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    int m_src_idx;
    // [[ev_def:field]]
    int m_dest_idx;
    MoveEnhancement() : MapLinkEvent(MapEventTypes::evMoveEnhancement)
    {}
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, 41);
        assert(false); // we don't send this, we receive it.
    }
    void    serializefrom(BitStream &bs) override
    {
        m_src_idx = bs.GetPackedBits(1);
        m_dest_idx = bs.GetPackedBits(1);
    }
    EVENT_IMPL(MoveEnhancement)
};

// [[ev_def:type]]
class SetEnhancement final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    uint32_t m_pset_idx;
    // [[ev_def:field]]
    uint32_t m_pow_idx;
    // [[ev_def:field]]
    uint32_t m_src_idx;
    // [[ev_def:field]]
    uint32_t m_dest_idx;
    SetEnhancement() : MapLinkEvent(evSetEnhancement)
    {}
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, 42);
        assert(false); // we don't send this, we receive it.
    }
    void    serializefrom(BitStream &bs) override
    {
        m_pset_idx = bs.GetPackedBits(1);
        m_pow_idx = bs.GetPackedBits(1);
        m_src_idx = bs.GetPackedBits(1);
        m_dest_idx = bs.GetPackedBits(1);
    }
    EVENT_IMPL(SetEnhancement)
};

// [[ev_def:type]]
class TrashEnhancement final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    int m_idx;
    TrashEnhancement() : MapLinkEvent(evTrashEnhancement)
    {}
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, 43);
        assert(false); // we don't send this, we receive it.
    }
    void    serializefrom(BitStream &bs) override
    {
        m_idx = bs.GetPackedBits(1);
    }
    EVENT_IMPL(TrashEnhancement)
};

// [[ev_def:type]]
class TrashEnhancementInPower final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    int m_pset_idx;
    // [[ev_def:field]]
    int m_pow_idx;
    // [[ev_def:field]]
    int m_eh_idx;
    TrashEnhancementInPower() : MapLinkEvent(MapEventTypes::evTrashEnhancementInPower)
    {}
    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, 44);
        assert(false); // we don't send this, we receive it.
    }
    void serializefrom(BitStream &bs) override
    {
        m_pset_idx  = bs.GetPackedBits(1);
        m_pow_idx   = bs.GetPackedBits(1);
        m_eh_idx = bs.GetPackedBits(1);
    }
    EVENT_IMPL(TrashEnhancementInPower)
};

// [[ev_def:type]]
class BuyEnhancementSlot final : public MapLinkEvent
{
public:
    // [[ev_def:field]]
    int m_num;
    // [[ev_def:field]]
    int m_pset_idx;
    // [[ev_def:field]]
    int m_pow_idx;
    BuyEnhancementSlot() : MapLinkEvent(MapEventTypes::evBuyEnhancementSlot)
    {}
    void    serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, 45);
        assert(false); // we don't send this, we receive it.
    }
    void    serializefrom(BitStream &bs) override
    {
        m_num       = bs.GetPackedBits(1);
        m_pset_idx  = bs.GetPackedBits(1);
        m_pow_idx   = bs.GetPackedBits(1);
    }
    EVENT_IMPL(BuyEnhancementSlot)

};

// [[ev_def:type]]
class RecvNewPower final : public MapLinkEvent
{
public:
                    // [[ev_def:field]]
    PowerPool_Info  ppool;
                    RecvNewPower() : MapLinkEvent(MapEventTypes::evRecvNewPower)
                    {}
    void            serializeto(BitStream &bs) const override
                    {
                        bs.StorePackedBits(1, 46);
                        assert(false); // we don't send this, we receive it.
                    }
    void            serializefrom(BitStream &bs) override
                    {
                        ppool.m_pcat_idx    = bs.GetPackedBits(3);
                        ppool.m_pset_idx    = bs.GetPackedBits(3);
                        ppool.m_pow_idx     = bs.GetPackedBits(3);
                    }
                    EVENT_IMPL(RecvNewPower)
};
} // end of SEGSEvents namespace
