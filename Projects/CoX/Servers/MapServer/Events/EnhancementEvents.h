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

class CombineEnhancements final : public MapLinkEvent
{
public:
    struct PowerEntry
    {
        uint32_t m_pset_idx;
        uint32_t m_pow_idx;
        uint32_t m_eh_idx;
    };
    PowerEntry first_power;
    PowerEntry second_power;
    CombineEnhancements() : MapLinkEvent(MapEventTypes::evCombineEnhancements)
    {}
    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1,40); // opcode
        assert(false); // since we will not send CombineEnhancements to anyone :)
    }
    // now to make this useful ;)

    void getPowerForCombine(BitStream &bs, PowerEntry &entry)
    {
        // first bit tells us if we have full/partial?? data
        // here we can do a small refactoring, because in both branches of if/else, the last set value is index.
        if(bs.GetBits(1))
        {
            entry.m_pset_idx = bs.GetPackedBits(1);
            entry.m_pow_idx = bs.GetPackedBits(1);
        }
        entry.m_eh_idx = bs.GetPackedBits(1);
    }
    void serializefrom(BitStream &bs)
    {
        getPowerForCombine(bs, first_power);
        getPowerForCombine(bs, second_power);
    }
};

class MoveEnhancement final : public MapLinkEvent
{
public:
    int m_src_idx;
    int m_dest_idx;
    MoveEnhancement() : MapLinkEvent(MapEventTypes::evMoveEnhancement)
    {}
    void    serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1, 41);
        assert(false); // we don't send this, we receive it.
    }
    void    serializefrom(BitStream &bs)
    {
        m_src_idx = bs.GetPackedBits(1);
        m_dest_idx = bs.GetPackedBits(1);
    }
};

class SetEnhancement final : public MapLinkEvent
{
public:
    uint32_t m_pset_idx;
    uint32_t m_pow_idx;
    uint32_t m_src_idx;
    uint32_t m_dest_idx;
    SetEnhancement() : MapLinkEvent(MapEventTypes::evSetEnhancement)
    {}
    void    serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1, 42);
        assert(false); // we don't send this, we receive it.
    }
    void    serializefrom(BitStream &bs)
    {
        m_pset_idx = bs.GetPackedBits(1);
        m_pow_idx = bs.GetPackedBits(1);
        m_src_idx = bs.GetPackedBits(1);
        m_dest_idx = bs.GetPackedBits(1);
    }
};

class TrashEnhancement final : public MapLinkEvent
{
public:
    int m_idx;
    TrashEnhancement() : MapLinkEvent(MapEventTypes::evTrashEnhancement)
    {}
    void    serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1, 43);
        assert(false); // we don't send this, we receive it.
    }
    void    serializefrom(BitStream &bs)
    {
        m_idx = bs.GetPackedBits(1);
    }
};
