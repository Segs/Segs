#include "SideWindow.h"
#include "ui_SideWindow.h"

#include "MapViewerApp.h"
#include "CoHSceneConverter.h"
#include "CoHModelLoader.h"
#include "CohModelConverter.h"
#include "DataPathsDialog.h"

#include <QtWidgets/QFileDialog>
#include <QtCore/QDebug>
#include <QtCore/QStringListModel>
#include <QStandardItemModel>
#include <Lutefisk3D/Scene/Node.h>
#include <Lutefisk3D/Graphics/Material.h>
#include <Lutefisk3D/Graphics/StaticModel.h>
#include <Lutefisk3D/Resource/JSONFile.h>
#include <Lutefisk3D/Resource/XMLFile.h>
#include <Lutefisk3D/IO/VectorBuffer.h>
using namespace Urho3D;
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
void SideWindow::onModelSelected(CoHNode *n,CoHModel *m, Urho3D::Drawable *d)
{
    if(!m) 
    {
        ui->txtModelName->setText("none");
        ui->txtTrickName->setText("none");
        ui->matDescriptionTxt->clear();
        return;
    }

    ui->matDescriptionTxt->clear();
    auto parts = m->name.split("__");
    ui->txtModelName->setText(parts.front());
    if(parts.size()>2)
        qDebug() << "Too many double underscores in name:"<<m->name;
    if(parts.size()==2)
        ui->txtTrickName->setText(parts.back());
    else
        ui->txtTrickName->setText("none");
    QString          matDesc;
    ConvertedGeoSet *geoset = m->geoset;
    StaticModel *    model  = (StaticModel *)d;
    matDesc += "<p>Geometries:<b>";
    QSet<void *> dumpedmats;
    for(int      i =0, fin =model->GetNumGeometries(); i<fin; ++i)
    {
        matDesc += "<p>";
        Material *material = model->GetMaterial(i);
        if(dumpedmats.contains(material))
            continue;
        dumpedmats.insert(material);

        XMLFile tgtf(d->GetContext());
        auto    root(tgtf.CreateRoot("material"));
        material->Save(root);
        matDesc += "Material:<b>"+ material->GetName()+"</b><br>";
        QString jsonbuf =tgtf.ToString("&nbsp;&nbsp;");
        jsonbuf         =jsonbuf.trimmed();
        jsonbuf.remove(0,21);
        jsonbuf.replace("<","[");
        jsonbuf.replace("/>","]");
        matDesc += "<p>"+ jsonbuf.replace("\n","<br>")+"</b><br>";

        matDesc += "</p>";
    }
    matDesc+="Textures in use<br>";
    for(TextureBind tbind : m->texture_bind_info) {
        matDesc += "Tex:<b>"+ geoset->tex_names[tbind.tex_idx]+"</b><br>";
    }
    ui->matDescriptionTxt->setHtml(matDesc);
    Urho3D::Node *     holder = d->GetNode();
    Matrix3x4          tr     = holder->GetTransform();
    Urho3D::Quaternion quat   = tr.Rotation();
    ui->pitchEdt->setText(QString::number(quat.PitchAngle()));
    ui->yawEdt->setText(QString::number(quat.YawAngle()));
    ui->rollEdt->setText(QString::number(quat.RollAngle()));
}
static QStandardItem * fillModel(CoHNode *node)
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
void SideWindow::onScenegraphLoaded(const CoHSceneGraph & sc)
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
    QString fl = QFileDialog::getOpenFileName(this,"Select a scenegraph .bin file to load",basepath+"/geobin",
        "All scenegraph formats(*.bin *.crl.*);;Scenegraphs (*.bin);;Cereal scenegraphs (*.crl.*)");
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
    CoHNode *n = (CoHNode *)v.value<void *>();
    emit nodeSelected(n);
    if(!n)
        return;

    QModelIndex parent_idx = m_model->parent(index);
    QVariant parentv = m_model->data(m_model->parent(index),Qt::UserRole);
    CoHNode *parent_node = (CoHNode *)parentv.value<void *>();
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
            CoHNode *n = (CoHNode *)val;
            emit nodeDisplayRequest(n,isroot.value<bool>());
            return;
        }
    }
    emit nodeDisplayRequest(nullptr,true);
}
