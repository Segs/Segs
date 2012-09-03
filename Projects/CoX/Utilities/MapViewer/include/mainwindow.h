#pragma once

#include <QMainWindow>
#include <osg/Node>

class CompositeViewerQOSG;
class MapModel;
class ViewQOSG;

namespace Ui {
    class MainWindow;
}
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
    CompositeViewerQOSG *m_viewer;
    osg::ref_ptr<ViewQOSG> view1;
};
