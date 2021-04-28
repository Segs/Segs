/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup Components
 * @{
 */

#include "Components/SEGSEvent.h"
#include "SEGSEventFactory.h"

using namespace SEGSEvents;
Finish *Finish::s_instance(new Finish());

const char *Event::info()
{
    return event_name(this->m_type);
}

//! @}
