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
class TradeOffer final : public GameCommandEvent
{
public:
    EVENT_IMPL(TradeOffer)
    TradeOffer();
    TradeOffer(uint32_t db_id, const QString& name);
    void serializeto(BitStream& bs) const override;

    // [[ev_def:field]]
    uint32_t m_db_id = 0;
    // [[ev_def:field]]
    QString m_name;
};


} // namespace SEGSEvents
