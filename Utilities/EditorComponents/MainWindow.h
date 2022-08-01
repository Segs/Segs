#pragma once

#include <QMainWindow>
#include <QMdiSubWindow>

namespace Ui
{
class MainWindow;
}

class SignalingMdiSubWindow : public QMdiSubWindow {
    Q_OBJECT
public:
    explicit SignalingMdiSubWindow(QWidget *parent = nullptr) : QMdiSubWindow(parent) {}

Q_SIGNALS:
    void closing();
public:
    void closeEvent(QCloseEvent *ev) override {
        widget()->setVisible(false);
        setWidget(nullptr); // we don't own the sub widget - nullify
        emit closing();
        QMdiSubWindow::closeEvent(ev);
    }
};

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
    QHash<int,QWidget *> m_editors;
    QHash<int,QMdiSubWindow *> m_mdis;
};
