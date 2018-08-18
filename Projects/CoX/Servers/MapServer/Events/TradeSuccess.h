/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
#pragma once

#include "GameCommandList.h"


class TradeSuccess final : public GameCommand
{
public:
    explicit TradeSuccess(const QString& msg);
    virtual ~TradeSuccess() override = default;
    virtual void serializeto(BitStream& bs) const override;

    const QString m_msg;
};
