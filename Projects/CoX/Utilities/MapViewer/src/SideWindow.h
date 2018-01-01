#ifndef SIDEWINDOW_H
#define SIDEWINDOW_H

#include <QtWidgets/QMainWindow>

#include <QAbstractItemModel>

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
struct CoHSceneGraph;
struct CoHNode;
struct CoHModel;

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
    void nodeSelected(CoHNode *n);
    void nodeDisplayRequest(CoHNode *n,bool isroot);
public slots:
    void onCameraPositionChanged(float x,float y,float z);
    void onModelSelected(CoHModel *m,Urho3D::Drawable *d);
    void onScenegraphLoaded(const CoHSceneGraph &sc);
private slots:
    void on_actionLoad_Scene_Graph_triggered();

    void on_actionSet_data_paths_triggered();

    void on_nodeList_clicked(const QModelIndex &index);

    void on_nodeList_doubleClicked(const QModelIndex &index);

private:
    MapViewerApp *m_map_viewer;
    Ui::SideWindow *ui;
    QAbstractItemModel *m_model;
};

#endif // SIDEWINDOW_H
