#include <QtGui>
#include <ace/ACE.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "objecttreewidget.h"
#include "mapmodel.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ObjectTreeWidget *tree=new ObjectTreeWidget(this);
    this->addDockWidget(Qt::LeftDockWidgetArea,tree);
    m_model=new MapModel(this);
    tree->set_model(m_model);

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
        if(false==m_model->read(selected_file))
        {
            QMessageBox::critical(this,
                                  tr("Map file error"),
                                  tr("Failed to open the selected map file\n(%1)").arg(selected_file)
                                  );
        }
    }
}
