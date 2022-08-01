#pragma once

#include <QWidget>


class QFileSystemModel;
namespace Ui
{
class AssetSelector;
}

class AssetSelector : public QWidget
{
    Q_OBJECT

public:
    explicit AssetSelector(QWidget *parent = nullptr);
    ~AssetSelector() override;

signals:
    void assetActivated(int type,const QString &path);
private slots:
    void on_fsTree_activated(const QModelIndex &index);
    void on_fsTree_doubleClicked(const QModelIndex &index);

private:
    Ui::AssetSelector *ui;
    QFileSystemModel *model;
};
