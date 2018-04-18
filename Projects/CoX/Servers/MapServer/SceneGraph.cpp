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
