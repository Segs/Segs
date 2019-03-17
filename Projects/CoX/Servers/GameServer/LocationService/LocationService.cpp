/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "LocationService.h"
#include "GameData/Entity.h"
#include "Messages/Map/MapEvents.h"
#include "Servers/MapServer/DataHelpers.h"
#include <QtCore/QDebug>

using namespace SEGSEvents;

ServiceToClientData* LocationService::on_location_visited(Entity* ent, Event *ev)
{
    LocationVisited* casted_ev = static_cast<LocationVisited *>(ev);
    qCDebug(logMapEvents()) << "Attempting a call to script location_visited with:"<<casted_ev->m_name<<qHash(casted_ev->m_name);

    //auto val = m_scripting_interface->callFuncWithClientContext(&session,"location_visited", qPrintable(casted_ev->m_name), casted_ev->m_pos);
    // sendInfoMessage(MessageChannel::DEBUG_INFO,qPrintable(casted_ev->m_name),session);

    qCWarning(logMapEvents) << "Unhandled location visited casted_event:" << casted_ev->m_name <<
                  QString("(%1,%2,%3)").arg(casted_ev->m_pos.x).arg(casted_ev->m_pos.y).arg(casted_ev->m_pos.z);

    ScriptingServiceToClientData* scriptData = new ScriptingServiceToClientData();
    scriptData->flags |= uint32_t(ScriptingServiceFlags::CallFuncWithClientContext);
    scriptData->funcName = "location_visited";
    scriptData->charArg = casted_ev->m_name;
    scriptData->locArg = casted_ev->m_pos;

    return new ServiceToClientData(ent, {scriptData}, casted_ev->m_name);
}

ServiceToClientData* LocationService::on_plaque_visited(Entity* ent, Event* ev)
{
    PlaqueVisited* casted_ev = static_cast<PlaqueVisited *>(ev);
    qCDebug(logMapEvents) << "Attempting a call to script plaque_visited with:"<<casted_ev->m_name<<qHash(casted_ev->m_name);

    //auto val = m_scripting_interface->callFuncWithClientContext(&session,"plaque_visited", qPrintable(casted_ev->m_name), casted_ev->m_pos);
    qCWarning(logMapEvents) << "Unhandled plaque visited casted_event:" << casted_ev->m_name <<
                  QString("(%1,%2,%3)").arg(casted_ev->m_pos.x).arg(casted_ev->m_pos.y).arg(casted_ev->m_pos.z);

    ScriptingServiceToClientData* scriptData = new ScriptingServiceToClientData();
    scriptData->flags |= uint32_t(ScriptingServiceFlags::CallFuncWithClientContext);
    scriptData->funcName = "plaque_visited";
    scriptData->charArg = casted_ev->m_name;
    scriptData->locArg = casted_ev->m_pos;

    return new ServiceToClientData(ent, {scriptData}, QString());
}
