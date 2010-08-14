#include "mapmodel.h"
#include <osg/Node>
#include <iostream>
MapModel::MapModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    m_scene.build_schema();
}

bool MapModel::read(QString file)
{
    bool res=m_scene.read(file.toStdString());
    build_osg_scene();
    return res;
}
void MapModel::build_osg_scene()
{
    SceneStorage::vDef::iterator iter=m_scene.m_root.begin();
    SceneStorage::vDef &arr(m_scene.m_root);
    while(iter!=arr.end())
    {
        std::cerr<<(*iter)->m_src_name;
        ++iter;
    }
}
QModelIndex MapModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row,column,0);
}
QModelIndex MapModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}
int MapModel::rowCount(const QModelIndex &parent) const
{
    return 0;
}
int MapModel::columnCount(const QModelIndex &parent) const
{
    return 0;
}
QVariant MapModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}
