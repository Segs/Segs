#include "AssetSelector.h"
#include "ui_AssetSelector.h"

AssetSelector::AssetSelector(QWidget *parent) :
      QWidget(parent),
      ui(new Ui::AssetSelector)
{
    ui->setupUi(this);
}

AssetSelector::~AssetSelector()
{
    delete ui;
}

void AssetSelector::on_fakeAsset_clicked()
{
    emit assetActivated(0,"data/bin/origins.bin");
}

