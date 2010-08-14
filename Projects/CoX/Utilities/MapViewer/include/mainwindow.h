#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}
class MapModel;
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
