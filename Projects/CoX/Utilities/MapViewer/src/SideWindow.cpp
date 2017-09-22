#include "SideWindow.h"
#include "ui_SideWindow.h"

#include "MapViewerApp.h"
#include "CoHSceneConverter.h"
#include "CohModelConverter.h"

#include <QtWidgets/QFileDialog>
#include <QtCore/QDebug>
extern QString basepath;
SideWindow::SideWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SideWindow)
{
    ui->setupUi(this);
    ui->txtModelName->setText("none");
    ui->txtTrickName->setText("none");
}

SideWindow::~SideWindow()
{
    delete ui;
}

void SideWindow::setMapViewer(MapViewerApp *mapview)
{
    m_map_viewer = mapview;
}

void SideWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void SideWindow::onCameraPositionChanged(float x, float y, float z)
{
    ui->labCamPos->setText(QString("x:%1 y:%2 z:%3")
                               .arg(x, 5, 'f', 2, QChar(' '))
                               .arg(y, 5, 'f', 2, QChar(' '))
                               .arg(z, 5, 'f', 2, QChar(' ')));
}
void SideWindow::onModelSelected(ConvertedModel *m, Urho3D::Drawable *d)
{
    if(m) 
    {
        auto parts = m->name.split("__");
        ui->txtModelName->setText(parts.front());
        if(parts.size()>2)
            qDebug() << "Too many double underscores in name:"<<m->name;
        if(parts.size()==2)
            ui->txtTrickName->setText(parts.back());
        else
            ui->txtTrickName->setText("none");
    }
    else
    {
        ui->txtModelName->setText("none");
        ui->txtTrickName->setText("none");
    }
}
void SideWindow::on_actionLoad_Scene_Graph_triggered()
{
    QString fl = QFileDialog::getOpenFileName(this,"Select a scenegraph .bin file to load",basepath,"Scenegraphs (*.bin)");
    emit scenegraphSelected(fl);
}
