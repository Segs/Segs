/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "LocationService.h"
#include "GameData/Entity.h"
#include "GameData/EntityHelpers.h"
#include "Messages/Map/MapEvents.h"
#include "Servers/MapServer/DataHelpers.h"
#include <QtCore/QDebug>

using namespace SEGSEvents;

std::unique_ptr<ServiceToClientData> LocationService::on_location_visited(Entity* ent, Event *ev)
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

    ScriptVector scripts {scriptData};
    return std::make_unique<ServiceToClientData>(ent, scripts, casted_ev->m_name);
}

std::unique_ptr<ServiceToClientData> LocationService::on_plaque_visited(Entity* ent, Event* ev)
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

    ScriptVector scripts {scriptData};
    return std::make_unique<ServiceToClientData>(ent, scripts, casted_ev->m_name);
}

std::unique_ptr<ServiceToClientData> LocationService::on_set_destination(Entity* ent, Event* ev)
{
    SetDestination* casted_ev = static_cast<SetDestination*>(ev);

    qCWarning(logMapEvents) << QString("SetDestination request: %1 <%2, %3, %4>")
                                .arg(casted_ev->point_index)
                                .arg(casted_ev->destination.x, 0, 'f', 1)
                                .arg(casted_ev->destination.y, 0, 'f', 1)
                                .arg(casted_ev->destination.z, 0, 'f', 1);

    // store destination, confirm accuracy and send back to client as waypoint.
    setCurrentDestination(*ent, casted_ev->point_index, casted_ev->destination);

    // this one should return a GameCommandEvent
    sendWaypoint(*ent->m_client, casted_ev->point_index, casted_ev->destination);

    return nullptr;
}
