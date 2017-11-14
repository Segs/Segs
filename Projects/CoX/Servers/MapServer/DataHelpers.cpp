#include "DataHelpers.h"

#include "MapServer.h"
#include "MapServerData.h"

#include <QtCore/QDebug>

int getEntityOriginIndex(bool is_player, const QString &origin_name)
{
    const MapServerData &data(g_GlobalMapServer->runtimeData());
    const Parse_AllOrigins &origins_to_search(is_player ? data.m_player_origins : data.m_other_origins);

    int idx=0;
    for(const Parse_Origin &orig : origins_to_search)
    {
        if(orig.Name.compare(origin_name,Qt::CaseInsensitive)==0)
            return idx;
        idx++;
    }
    qWarning() << "Failed to locate origin index for"<<origin_name;
    return 0;
}

int getEntityClassIndex(bool is_player, const QString &class_name)
{
    const MapServerData &data(g_GlobalMapServer->runtimeData());
    const Parse_AllCharClasses &classes_to_search(is_player ? data.m_player_classes : data.m_other_classes);

    int idx=0;
    for(const CharClass_Data &classdata : classes_to_search)
    {
        if(classdata.m_Name.compare(class_name,Qt::CaseInsensitive)==0)
            return idx;
        idx++;
    }
    qWarning() << "Failed to locate class index for"<<class_name;
    return 0;

}
