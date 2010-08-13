#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractItemModel>
#include <QModelIndex>
#include "MapStructure.h"

namespace Ui {
    class MainWindow;
}
// TODO: fully wrap SceneStorage
class MapModel : public QAbstractItemModel
{
    SceneStorage m_scene;
public:
    MapModel(QObject *parent) : QAbstractItemModel(parent)
    {
        m_scene.build_schema();
    }
    bool read(QString file)
    {
        bool res=m_scene.read(file.toStdString());
        return res;
    }
    QModelIndex index(int row, int column, const QModelIndex &parent) const
    {
        return createIndex(row,column,0);
    }
    QModelIndex parent(const QModelIndex &child) const
    {
        return QModelIndex();
    }
    int rowCount(const QModelIndex &parent) const
    {
        return 0;
    }
    int columnCount(const QModelIndex &parent) const
    {
        return 0;
    }
    QVariant data(const QModelIndex &index, int role) const
    {
        return QVariant();
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void finish_viewer();
    void load_map();
private:
    Ui::MainWindow *ui;
    MapModel *m_model;
};

#endif // MAINWINDOW_H
