/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef SIDEWINDOW_H
#define SIDEWINDOW_H

#include <QtWidgets/QMainWindow>

#include <QAbstractItemModel>

namespace Ui
{
    class SideWindow;
}

namespace Urho3D
{
    class Node;
    class Scene;
    class StaticModel;
    class Vector3;
    class Drawable;
}
namespace SEGS
{
struct SceneNode;
struct SceneGraph;
struct Model;
struct RootNode;
}

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
    void nodeSelected(SEGS::SceneNode *n);
    void nodeDisplayRequest(SEGS::SceneNode *n,bool isroot);
    void refDisplayRequest(SEGS::RootNode *n,bool show_all);
public slots:
    void onCameraPositionChanged(float x,float y,float z);
    void onModelSelected(SEGS::SceneNode *n, SEGS::Model *m, Urho3D::Drawable*d);
    void onScenegraphLoaded(const SEGS::SceneGraph &sc);
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
