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

void MainWindow::onAssetActivated(int type, const QString &path)
{

    if (!m_editors[type])
    {
        auto *ed = new SegsOriginEditor();
        ed->setAttribute(Qt::WA_DeleteOnClose, false);
        m_editors[type] = ed;
    }

    if (m_editors[type]->isVisible())
    {
        m_mdis[type]->activateWindow();
    }
    else
    {
        QMdiSubWindow *mdi;
        if(!m_mdis.contains(type))
        {
            mdi    = new SignalingMdiSubWindow(ui->centralwidget);
            m_mdis[type] = mdi;
        }
        else
            mdi = m_mdis[type];
        mdi->setWidget(m_editors[type]);
        m_editors[type]->setVisible(true);
        mdi->showMaximized();
        mdi->setAttribute(Qt::WA_DeleteOnClose,false);
    }
}

void MainWindow::addEditors() {
    // TODO: register a bunch of asset_type->editor_creation_function mappings
}

void MainWindow::addMdis() {
    //m_mdis.push_back(new QMdiSubWindow(ui->centralwidget));
    //m_mdis.back()->setAttribute(Qt::WA_DeleteOnClose,false);
    //subWindow1->setWidget(w);
}

MainWindow::~MainWindow()
{
    delete ui;
}
