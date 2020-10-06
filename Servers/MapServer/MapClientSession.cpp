/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "MapClientSession.h"
#include "Components/Logging.h"

void MapClientSession::addCommandToSendNextUpdate(std::unique_ptr<SEGSEvents::GameCommandEvent> &&v)
{
    m_contents.emplace_back(std::move(v));
}

void MapClientSession::AddShortcut(int index, NetCommand *command)
{
    if(m_shortcuts.find(index) != m_shortcuts.end())
    {
        qDebug() << "Replacing command" << index << m_shortcuts[index]->m_name <<
                    "->" << command->m_name;
    }
    m_shortcuts[index] = command;
}
//! @}

