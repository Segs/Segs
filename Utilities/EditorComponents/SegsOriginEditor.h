#pragma once

#include <QtWidgets/QWidget>

namespace Ui {
class SegsOriginEditor;
}

class QAbstractItemModel;

class SegsOriginEditor : public QWidget
{
    Q_OBJECT

public:
    explicit SegsOriginEditor(QWidget *parent = nullptr);
    ~SegsOriginEditor();

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_actionOpen_triggered();

    void on_actionSave_triggered();

    void on_OriginPowerslistView_activated(const QModelIndex &index);

private:
    Ui::SegsOriginEditor *ui;
    QAbstractItemModel *m_origins;
};

