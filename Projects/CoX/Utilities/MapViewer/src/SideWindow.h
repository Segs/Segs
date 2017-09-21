#ifndef SIDEWINDOW_H
#define SIDEWINDOW_H

#include <QtWidgets/QMainWindow>

namespace Ui {
class SideWindow;
}
namespace Urho3D {
class Node;
class Scene;
class StaticModel;
class Vector3;
class Drawable;
}
struct ConvertedModel;

class MapViewerApp;
class SideWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SideWindow(QWidget *parent = nullptr);
    ~SideWindow();
    void setMapViewer(MapViewerApp *mapview);
protected:
    void changeEvent(QEvent *e);
signals:
    void scenegraphSelected(const QString &path);
public slots:
    void onCameraPositionChanged(float x,float y,float z);
private slots:
    void on_actionLoad_Scene_Graph_triggered();

private:
    MapViewerApp *m_map_viewer;
    Ui::SideWindow *ui;
};

#endif // SIDEWINDOW_H
