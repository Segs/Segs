/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup Components
 * @{
 */

#include "SEGSEvent.h"

SEGSEvent SEGSEvent::s_ev_finish(SEGS_EventTypes::evFinish,nullptr);

const char *SEGSEvent::info()
{
    return typeid(*this).name();
}

//! @}
