#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <osg/Node>

#include "MapStructure.h"

// TODO: fully wrap SceneStorage
class MapModel : public QAbstractItemModel
{
    SceneStorage m_scene;
    Q_OBJECT
    osg::ref_ptr<osg::Node> build_osg_scene();
public:
    explicit MapModel(QObject *parent=0);
    osg::ref_ptr<osg::Node> read(QString file);
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool hasChildren(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
protected:
};
