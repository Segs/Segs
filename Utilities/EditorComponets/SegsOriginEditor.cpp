#include "SegsOriginEditor.h"
#include "ui_SegsOriginEditor.h"

#include <QStringListModel>
#include <QDebug>


SegsOriginEditor::SegsOriginEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SegsOriginEditor)
{
    ui->setupUi(this);
    auto origin_list = new QStringListModel(this);
    m_origins = origin_list;
    origin_list->setStringList({"Test","Test2"});

    ui->OriginPowerslistView->setModel(m_origins);
}

SegsOriginEditor::~SegsOriginEditor()
{
    delete ui;
}

void SegsOriginEditor::changeEvent(QEvent *e)
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

void SegsOriginEditor::on_actionOpen_triggered()
{

}


void SegsOriginEditor::on_actionSave_triggered()
{

}


void SegsOriginEditor::on_OriginPowerslistView_activated(const QModelIndex &index)
{
    if(index.isValid()) {
        qDebug() << "Model entry selected" << m_origins->data(index).toString();
    }
}

