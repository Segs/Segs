#pragma once

#include <QMainWindow>

class QMdiSubWindow;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAssetActivated(int type, const QString &path);

private:
    void addDocks();
    void addMdis();
    void addEditors();

    Ui::MainWindow        *ui;
    std::vector<QWidget *> m_editors;
    std::vector<QMdiSubWindow *> m_mdis;
};
