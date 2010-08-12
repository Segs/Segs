#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <ace/ACE.h>
#include "MapStructure.h"
#include "objecttreewidget.h"
void test_reading()
{
    SceneStorage m_scene;
    m_scene.build_schema();
    bool res=m_scene.read("./data/City_00_01.bin");
    if(res)
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Done.\n") ));
    else
        ACE_DEBUG((LM_WARNING,ACE_TEXT("Failed!\n") ));
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->addDockWidget(Qt::LeftDockWidgetArea,new ObjectTreeWidget(this));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::finish_viewer()
{
    qApp->exit(0);
}
