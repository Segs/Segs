#ifndef MAPMODEL_H
#define MAPMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include "MapStructure.h"

// TODO: fully wrap SceneStorage
class MapModel : public QAbstractItemModel
{
    SceneStorage m_scene;
    Q_OBJECT
    void build_osg_scene();
public:
    explicit MapModel(QObject *parent=0);
    bool read(QString file);
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
};

#endif // MAPMODEL_H
