#include "MapViewerApp.h"
#include "CohTextureConverter.h"

#include <Lutefisk3D/Engine/EngineDefs.h>

#include <QSettings>
#include <QtCore/QDebug>
#include <QtWidgets/QApplication>

using namespace Urho3D;

// Stupid global to allow converters to know where the unpacked files are
QString basepath;

MapViewerApp::MapViewerApp(Context * ctx) : Application("CoX Map Viewer",ctx)
{
}
MapViewerApp::~MapViewerApp()
{
}
void MapViewerApp::Setup()
{
    engineParameters_[EP_FULL_SCREEN]  = false;
    engineParameters_[EP_WINDOW_TITLE] = m_appName;

}
void MapViewerApp::Start()
{
    QSettings our_settings(QSettings::IniFormat,QSettings::UserScope,"SEGS","MapViewer");
    basepath = our_settings.value("ExtractedDir",QString()).toString();
    if(!basepath.endsWith('/'))
        basepath+='/';

    preloadTextureNames();

}
