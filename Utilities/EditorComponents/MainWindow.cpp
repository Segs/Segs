#include "MainWindow.h"

#include "ui_MainWindow.h"

#include "AssetSelector.h"
#include "SegsOriginEditor.h"

#include <QDockWidget>
#include <QMdiSubWindow>

MainWindow::MainWindow(QWidget *parent) :
      QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    addEditors();
    addDocks();
    addMdis();

}

void MainWindow::addDocks() {
    QDockWidget *dock = new QDockWidget(tr("Asset List"), this);
    dock->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
    auto *sel = new AssetSelector(dock);
    dock->setWidget(sel);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    connect(sel,&AssetSelector::assetActivated,this,&MainWindow::onAssetActivated);

}

void MainWindow::onAssetActivated(int type, const QString &path) {

    if(m_editors[0]->isVisible()) {
        m_mdis[0]->activateWindow();
        m_editors[0]->activateWindow();
    } else {
        m_editors[0]->showMaximized();
    }
}

void MainWindow::addEditors() {
    m_editors.push_back(new SegsOriginEditor());
    m_editors.back()->setAttribute(Qt::WA_DeleteOnClose,false);

}

void MainWindow::addMdis() {
    m_mdis.push_back(new QMdiSubWindow(ui->centralwidget));
    m_mdis.back()->setAttribute(Qt::WA_DeleteOnClose,false);
    //subWindow1->setWidget(w);
}

MainWindow::~MainWindow()
{
    delete ui;
}
