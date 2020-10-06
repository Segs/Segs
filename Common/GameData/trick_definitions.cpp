#include "trick_definitions.h"
#include "Common/GameData/anim_definitions.h"

#include <QDebug>

GeometryModifiers *findGeomModifier(SceneModifiers &tricks,const QString &modelname, const QString &trick_path)
{
    QStringList parts = modelname.split("__");
    if( parts.size()<2 )
        return nullptr;
    parts.removeFirst();
    QString bone_trick_name = parts.join("__");
    GeometryModifiers *result = tricks.g_tricks_string_hash_tab.value(bone_trick_name.toLower(),nullptr);
    if( result )
        return result;
    qDebug() << "Can't find modifier for" << trick_path<<modelname;
    return nullptr;
}
