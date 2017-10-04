#include "SideWindow.h"
#include "ui_SideWindow.h"

#include "MapViewerApp.h"
#include "CoHSceneConverter.h"
#include "CohModelConverter.h"
#include "DataPathsDialog.h"

#include <QtWidgets/QFileDialog>
#include <QtCore/QDebug>
#include <QtCore/QStringListModel>
#include <QStandardItemModel>
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
static QStandardItem * fillModel(ConvertedNode *node)
{
    QStandardItem *root = new QStandardItem;
    root->setData(node->name + ": Children("+QString::number(node->children.size())+")",Qt::DisplayRole);
    root->setData(QVariant::fromValue((void*)node),Qt::UserRole);
    root->setData(QVariant::fromValue(false),Qt::UserRole+1); // not a root node

    for(auto child : node->children)
    {
        QStandardItem *rowItem = fillModel(child.m_def);
        root->appendRow(rowItem);
    }
    return root;
}
void SideWindow::onScenegraphLoaded(const ConvertedSceneGraph & sc)
{
    QStandardItemModel *m=new QStandardItemModel();
    QStandardItem *item = m->invisibleRootItem();
    for(auto ref : sc.refs)
    {
        QStandardItem *rowItem = new QStandardItem;
        rowItem->setData(ref->node->name,Qt::DisplayRole);
        rowItem->setData(QVariant::fromValue((void*)ref->node),Qt::UserRole);
        rowItem->setData(QVariant::fromValue(true),Qt::UserRole+1);
        item->appendRow(rowItem);
    }
    if(sc.refs.empty())  // no scene graph roots in the graph
    {
        QSet<void *> ischild;
        for(auto node : sc.all_converted_defs)
        {
            for(auto chld : node->children)
                ischild.insert(chld.m_def);
        }
        for(auto node : sc.all_converted_defs) {
            if(ischild.contains(node))
                continue;
            QStandardItem *rowItem = fillModel(node);
            item->appendRow(rowItem);
        }
    }
    ui->nodeList->setModel(m);
    m_model = m;
}
void SideWindow::on_actionLoad_Scene_Graph_triggered()
{
    QString fl = QFileDialog::getOpenFileName(this,"Select a scenegraph .bin file to load",basepath+"/geobin","Scenegraphs (*.bin)");
    emit scenegraphSelected(fl);
}

void SideWindow::on_actionSet_data_paths_triggered()
{
    DataPathsDialog dlg;
    dlg.exec();
}

void SideWindow::on_nodeList_clicked(const QModelIndex &index)
{
    QVariant v=m_model->data(index,Qt::UserRole);
    ConvertedNode *n = (ConvertedNode *)v.value<void *>();
    emit nodeSelected(n);
    if(!n)
        return;

    QModelIndex parent_idx = m_model->parent(index);
    QVariant parentv = m_model->data(m_model->parent(index),Qt::UserRole);
    ConvertedNode *parent_node = (ConvertedNode *)parentv.value<void *>();
    if(!parent_node)
        return;
    for(auto chld : parent_node->children)
    {
        if(chld.m_def==n) {
            Urho3D::Quaternion quat=chld.m_matrix.Rotation();
            ui->pitchEdt->setText(QString::number(quat.PitchAngle()));
            ui->yawEdt->setText(QString::number(quat.YawAngle()));
            ui->rollEdt->setText(QString::number(quat.RollAngle()));
            break;
        }
    }
}

void SideWindow::on_nodeList_doubleClicked(const QModelIndex &index)
{
    QVariant v=m_model->data(index,Qt::UserRole);
    QVariant isroot=m_model->data(index,Qt::UserRole+1);
    if(v.isValid() && isroot.isValid())
    {
        void *val = v.value<void *>();
        if(val) {
            ConvertedNode *n = (ConvertedNode *)val;
            emit nodeDisplayRequest(n,isroot.value<bool>());
            return;
        }
    }
    emit nodeDisplayRequest(nullptr,true);
}
