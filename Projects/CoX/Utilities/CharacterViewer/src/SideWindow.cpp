/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup CharViewer Projects/CoX/Utilities/CharacterViewer
 * @{
 */

#include "SideWindow.h"
#include "ui_SideWindow.h"

#include "Common/Runtime/Prefab.h"
#include "CharViewerApp.h"

#include "CoHModelLoader.h"
#include "CohModelConverter.h"
#include "DataPathsDialog.h"
#include "Common/GameData/GameDataStore.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <QtWidgets/QFileDialog>
#include <QtCore/QDebug>
#include <QtWidgets/QMessageBox>
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
    initNpcList();
}

SideWindow::~SideWindow()
{
    delete ui;
}

void SideWindow::setMapViewer(CharViewerApp *mapview)
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

void SideWindow::onModelSelected(SEGS::SceneNode * /*n*/, SEGS::Model *m, Urho3D::Drawable *d)
{
    if(!m)
    {
        ui->txtName->setText(QStringLiteral("none"));
        ui->txtDisplayName->setText(QStringLiteral("none"));
        ui->txtClass->setText(QStringLiteral("none"));
        ui->matDescriptionTxt->clear();
        return;
    }

    ui->matDescriptionTxt->clear();
//    auto parts = m->name.split(QStringLiteral("__"));
//    ui->txtModelName->setText(parts.front());

//    if(parts.size()>2)
//        qDebug() << "Too many double underscores in name:"<<m->name;
//    if(parts.size()==2)
//        ui->txtTrickName->setText(parts.back());
//    else
//        ui->txtTrickName->setText("none");
//    QString          matDesc;
//    SEGS::GeoSet *geoset = m->geoset;
//    StaticModel *    model  = (StaticModel *)d;
//    matDesc += "<p>Geometries:<b>";
//    QSet<void *> dumpedmats;
//    for(int      i =0, fin =model->GetNumGeometries(); i<fin; ++i)
//    {
//        matDesc += "<p>";
//        Material *material = model->GetMaterial(i);
//        if(dumpedmats.contains(material))
//            continue;
//        dumpedmats.insert(material);

//        XMLFile tgtf(d->GetContext());
//        auto    root(tgtf.CreateRoot("material"));
//        material->Save(root);
//        matDesc += "Material:<b>"+ material->GetName()+"</b><br>";
//        QString jsonbuf =tgtf.ToString("&nbsp;&nbsp;");
//        jsonbuf         =jsonbuf.trimmed();
//        jsonbuf.remove(0,21);
//        jsonbuf.replace("<","[");
//        jsonbuf.replace("/>","]");
//        matDesc += "<p>"+ jsonbuf.replace("\n","<br>")+"</b><br>";

//        matDesc += "</p>";
//    }
//    matDesc+="Textures in use<br>";
//    for(SEGS::TextureBind tbind : m->texture_bind_info) {
//        matDesc += "Tex:<b>"+ geoset->tex_names[tbind.tex_idx]+"</b><br>";
//    }
//    ui->matDescriptionTxt->setHtml(matDesc);
//    Urho3D::Node *     holder = d->GetNode();
//    Matrix3x4          tr     = holder->GetTransform();
//    Urho3D::Quaternion quat   = tr.Rotation();
//    ui->pitchEdt->setText(QString::number(quat.PitchAngle()));
//    ui->yawEdt->setText(QString::number(quat.YawAngle()));
//    ui->rollEdt->setText(QString::number(quat.RollAngle()));
//    Urho3D::Node *iter = holder;
//    QString prop_text;
//    while(iter)
//    {
//        Variant stored = iter->GetVar("CoHNode");
//        if(stored!=Variant::EMPTY)
//        {
//            CoHNode * cn = (CoHNode *)stored.GetVoidPtr();
//            if(cn->m_properties)
//            {
//                prop_text += QString("%1\n").arg(cn->m_name);
//                for(const GroupProperty_Data &prop : *cn->m_properties)
//                {
//                    prop_text += QString("%2 - %3 [%4]\n")
//                            .arg(QString(prop.propName))
//                            .arg(QString(prop.propValue))
//                            .arg(prop.propertyType);
//                }
//            }
//        }
//        iter = iter->GetParent();
//    }
//    ui->propertiesTxt->setPlainText(prop_text);
}

static QStandardItem * fillModel(CoHNode *node)
{
    QStandardItem *root = new QStandardItem;
    root->setData(node->m_name + ": Children("+QString::number(node->m_children.size())+")",Qt::DisplayRole);
    root->setData(QVariant::fromValue((void*)node),Qt::UserRole);
    root->setData(QVariant::fromValue(false),Qt::UserRole+1); // not a root node

    for(const SEGS::SceneNodeChildTransform &child : node->m_children)
    {
        QStandardItem *rowItem = fillModel(child.node);
        root->appendRow(rowItem);
    }
    return root;
}
static QStandardItem *npcEntry(const Parse_NPC &c)
{
    QStandardItem *root = new QStandardItem;
    root->setData(c.m_Name.isEmpty() ? c.m_DisplayName : c.m_Name,Qt::DisplayRole);
    root->setData(QVariant::fromValue((void*)&c),Qt::UserRole);
    root->setData(QVariant::fromValue(true),Qt::UserRole+1);

    return root;
}
void SideWindow::initNpcList()
{
    const NPCStorage &npc_costumes(getGameData().getNPCDefinitions());

    QStandardItemModel *m=new QStandardItemModel();
    QStandardItem *item = m->invisibleRootItem();
    QStandardItem *npcCostumeRoot = new QStandardItem;

    npcCostumeRoot->setData("Npc costumes",Qt::DisplayRole);
    item->appendRow(npcCostumeRoot);
    QByteArray current_prefix;
    QStandardItem *prefix_root=nullptr;
    for(const Parse_NPC &npc : npc_costumes.m_all_npcs)
    {
        auto parts=npc.m_Name.split('_');
        QStandardItem *rowItem = npcEntry(npc);
        if(parts.size()>1)
        {
            if(current_prefix != parts.front())
            {
                prefix_root = new QStandardItem;
                current_prefix = parts.front();
                prefix_root->setData(current_prefix,Qt::DisplayRole);
                npcCostumeRoot->appendRow(prefix_root);
            }
            prefix_root->appendRow(rowItem);
        }
        else
            npcCostumeRoot->appendRow(rowItem);
    }
    ui->nodeList->setModel(m);
    delete m_model; // free old model if any
    m_model = m;
}

void SideWindow::on_actionSet_data_paths_triggered()
{
    DataPathsDialog dlg;
    dlg.exec();
}
void SideWindow::fillCostumesTab(const std::vector<Parse_Costume> &costumes)
{
    QStandardItemModel *m=new QStandardItemModel();
    QStandardItem *item = m->invisibleRootItem();
    auto prev_model = m_costume_model;
    int idx=0;
    for(const Parse_Costume &costume : costumes)
    {
        QStandardItem *costumeRoot = new QStandardItem;
        costumeRoot->setData(QString("Costume %1").arg(idx++),Qt::DisplayRole);
        costumeRoot->setData(QVariant::fromValue((void*)&costume),Qt::UserRole);
        item->appendRow(costumeRoot);
    }

    m_costume_model = m;
    ui->treeCostumes->setModel(m);
    delete prev_model;
}
void SideWindow::on_nodeList_clicked(const QModelIndex &index)
{
    QVariant v=m_model->data(index,Qt::UserRole);
    QVariant isnpc=m_model->data(index,Qt::UserRole+1);
    const Parse_NPC *npc = (const Parse_NPC *)v.value<void *>();
    if(isnpc.isValid() && isnpc.toBool())
    {
        // fill the costumes and powers tabs.
        fillCostumesTab(npc->m_Costumes);
//        emit costumeSelected(npc);
    }
    else
        ;//        emit costumeSelected(nullptr);
    if(!npc)
        return;

    ui->txtDisplayName->setText(npc->m_DisplayName);
    ui->txtName->setText(npc->m_Name);
    ui->txtClass->setText(npc->m_Class);
    ui->edtRank->setValue(npc->m_Rank);
    ui->edtLevel->setValue(npc->m_Level);
    ui->edtXp->setValue(npc->m_XP);
}

void SideWindow::on_nodeList_doubleClicked(const QModelIndex &index)
{
    QVariant v=m_model->data(index,Qt::UserRole);
    QVariant isroot=m_model->data(index,Qt::UserRole+1);
    if(v.isValid() && isroot.isValid())
    {
        void *val = v.value<void *>();
        if(val)
        {
            if(isroot.toBool())
            {
            }
            else
            {
                Parse_NPC *n = (Parse_NPC *)val;
//              emit nodeDisplayRequest(n,isroot.value<bool>());
            }
            return;
        }
    }
    emit nodeDisplayRequest(nullptr,true);
}

void SideWindow::on_actionLoad_Costume_triggered()
{
    QMessageBox::warning(this,"Warning","We do not have a player costume reader yet.");
}

//! @}

void SideWindow::on_treeCostumes_clicked(const QModelIndex &index)
{
    QVariant v=m_costume_model->data(index,Qt::UserRole);
    const Parse_Costume *costume = (const Parse_Costume *)v.value<void *>();
    if(costume)
    {
        emit costumeSelected(costume);
    }
}
