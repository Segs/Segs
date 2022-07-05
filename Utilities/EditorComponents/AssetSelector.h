#pragma once

#include <QWidget>

namespace Ui
{
class AssetSelector;
}

class AssetSelector : public QWidget
{
    Q_OBJECT

public:
    explicit AssetSelector(QWidget *parent = nullptr);
    ~AssetSelector();

signals:
    void assetActivated(int type,const QString &path);
private slots:
    void on_fakeAsset_clicked();

private:
    Ui::AssetSelector *ui;
};
