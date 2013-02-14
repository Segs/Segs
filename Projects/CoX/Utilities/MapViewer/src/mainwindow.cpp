#include <QtGui>
#include <QtCore>
#include <ace/ACE.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "objecttreewidget.h"
#include "mapmodel.h"
#include "QOSGWidget.h"
#include "CompositeViewerQOSG.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ObjectTreeWidget *tree=new ObjectTreeWidget(this);
    this->addDockWidget(Qt::LeftDockWidgetArea,tree);
    m_viewer = ui->m_viewer;
    m_model=new MapModel(this);
    tree->set_model(m_model);

    view1 = new ViewQOSG(ui->m_view);
    m_viewer->addView(view1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::finish_viewer()
{
    qApp->exit(0);
}
void MainWindow::load_map()
{
    QFileDialog dlg(this,tr("Select map file to load."),".");
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setNameFilter("CoX Map files (*.bin)");
    if(dlg.exec())
    {
        QString selected_file=dlg.selectedFiles()[0];
        osg::ref_ptr<osg::Node> scn = m_model->read(selected_file);
        if(!scn.valid())
        {
            QMessageBox::critical(this,
                                  tr("Map file error"),
                                  tr("Failed to open the selected map file\n(%1)").arg(selected_file)
                                  );
        }
        view1->setSceneData(scn);
    }
}
