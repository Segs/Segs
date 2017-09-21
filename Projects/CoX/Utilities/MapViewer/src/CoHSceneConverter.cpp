#include "CoHSceneConverter.h"

#include "GameData/DataStorage.h"
#include "GameData/scenegraph_serializers.h"
#include "GameData/scenegraph_definitions.h"
#include "GameData/trick_definitions.h"
#include "GameData/trick_serializers.h"

#include <Lutefisk3D/Core/Context.h>
#include <Lutefisk3D/Graphics/StaticModel.h>
#include <Lutefisk3D/Graphics/Texture.h>
#include <Lutefisk3D/Math/BoundingBox.h>
#include <Lutefisk3D/Math/Matrix3x4.h>
#include <Lutefisk3D/Resource/Image.h>
#include <Lutefisk3D/Resource/ResourceCache.h>
#include <Lutefisk3D/Scene/Node.h>
#include <Lutefisk3D/UI/Font.h>
#include <Lutefisk3D/UI/Text3D.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <vector>

using namespace Urho3D;
extern QString basepath;

namespace
{
AllTricks_Data g_tricks_store;
QHash<QString,GeometryModifiers *> tricks_string_hash_tab;
QString buildBaseName(QString path) 
{
    QStringList z = path.split(QDir::separator());

    if(z.size()>1)
        z.pop_back(); // remove file name
    if(z.front()=="object_library")
        z.pop_front();
    else if(z.contains("maps",Qt::CaseInsensitive))
    {
        while(0!=z.front().compare("maps",Qt::CaseInsensitive))
            z.pop_front();
    }
    return z.join('/');

}
QString mapNameToPath(const QString &a1)
{

    int start_idx = a1.indexOf("object_library",Qt::CaseInsensitive);
    if ( -1==start_idx )
        start_idx = a1.indexOf("maps",Qt::CaseInsensitive);
    QString buf = basepath+"geobin/" + a1.mid(start_idx);
    int last_dot = buf.lastIndexOf('.');
    if(-1==last_dot)
        buf+=".bin";
    else
        buf.replace(last_dot,buf.size()-last_dot,".bin");
    return buf;
}
}
bool loadSceneGraph(ConvertedSceneGraph &conv,const QString &path)
{
    QString binName = mapNameToPath(path);
    return true;
}
