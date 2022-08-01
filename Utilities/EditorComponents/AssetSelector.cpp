#include "AssetSelector.h"
#include "ui_AssetSelector.h"

#include <QFileSystemModel>
#include <QDebug>

AssetSelector::AssetSelector(QWidget *parent) :
      QWidget(parent),
      ui(new Ui::AssetSelector)
{
    ui->setupUi(this);
    model = new QFileSystemModel(this);
    model->setRootPath(QDir::currentPath());
    ui->fsTree->setModel(model);
    ui->fsTree->setRootIndex(model->index(QDir::currentPath()));
}

AssetSelector::~AssetSelector()
{
    delete ui;
    delete model;
}

void AssetSelector::on_fsTree_activated(const QModelIndex &index)
{
    qDebug() << "Asset selector activated"<<model->filePath(index);
}


void AssetSelector::on_fsTree_doubleClicked(const QModelIndex &index)
{
    QString fname = model->filePath(index);
    QDir from_dir(model->rootPath());
    QString rel_path = from_dir.relativeFilePath(fname);
    qDebug() << rel_path;
    int asset_type = -1; // unknown asset
    if(rel_path=="data/bin/origins.bin") {
        asset_type = 0;
    }
    emit assetActivated(asset_type,rel_path);
}

