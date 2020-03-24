/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
#pragma once

#include "GameCommand.h"

#include <QtCore/QString>


namespace SEGSEvents
{


// [[ev_def:type]]
class TradeSuccess final : public GameCommandEvent
{
public:
    EVENT_IMPL(TradeSuccess)
    TradeSuccess();
    explicit TradeSuccess(const QString& msg);
    void serializeto(BitStream& bs) const override;

    // [[ev_def:field]]
    QString m_msg;
};


} // namespace SEGSEvents
