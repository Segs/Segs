/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "SceneGraph.h"
#include "Logging.h"
SceneGraph::SceneGraph()
{

}

bool SceneGraph::loadFromFile(const QString &)
{
    qCCritical(logMapEvents) << "Loading scene graphs not implemented yet";
    return false;
}

//! @}
