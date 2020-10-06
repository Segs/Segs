/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServerEvents Projects/CoX/Servers/MapServer/Events
 * @{
 */

#include "WindowState.h"

#include "Components/BitStream.h"
using namespace SEGSEvents;

void WindowState::serializeto(BitStream &bs) const
{
    bs.StorePackedBits(1,14); // opcode
}

void WindowState::serializefrom(BitStream &bs)
{
    wnd.m_idx = (WindowIDX)bs.GetPackedBits(1);

    wnd.m_posx = bs.GetPackedBits(1);
    wnd.m_posy = bs.GetPackedBits(1);
    wnd.m_mode = (WindowVisibility)bs.GetPackedBits(1);
    wnd.m_locked = bs.GetPackedBits(1);
    wnd.m_color = bs.GetPackedBits(1);
    wnd.m_alpha = bs.GetPackedBits(1);

    if((wnd.m_draggable_frame = bs.GetBits(1)))
    {
        wnd.m_width = bs.GetPackedBits(1);
        wnd.m_height = bs.GetPackedBits(1);
    }
}

//! @}
