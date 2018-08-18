/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
#pragma once

#include "GameCommandList.h"


class TradeOffer final : public GameCommand
{
public:
    TradeOffer(uint32_t db_id, const QString& name);
    virtual ~TradeOffer() override = default;
    virtual void serializeto(BitStream& bs) const override;

    const uint32_t m_db_id;
    const QString m_name;
};
