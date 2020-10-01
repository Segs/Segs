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

class CharViewerApp;

class SideWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SideWindow(QWidget *parent = nullptr);
    ~SideWindow();
    void setMapViewer(CharViewerApp *mapview);
protected:
    void changeEvent(QEvent *e);
    void initNpcList();
    void fillCostumesTab(const std::vector<struct Parse_Costume> &costumes);
signals:
    void scenegraphSelected(const QString &path);
    void costumeSelected(const struct Parse_Costume *n);
    void nodeDisplayRequest(SEGS::SceneNode *n,bool isroot);
public slots:
    void onModelSelected(SEGS::SceneNode *n, SEGS::Model *m, Urho3D::Drawable*d);
private slots:
    void on_actionSet_data_paths_triggered();
    void on_nodeList_clicked(const QModelIndex &index);
    void on_nodeList_doubleClicked(const QModelIndex &index);
    void on_actionLoad_Costume_triggered();

    void on_treeCostumes_clicked(const QModelIndex &index);

private:
    CharViewerApp *m_map_viewer;
    Ui::SideWindow *ui;
    QAbstractItemModel *m_model=nullptr;
    QAbstractItemModel *m_costume_model=nullptr;
};

#endif // SIDEWINDOW_H
