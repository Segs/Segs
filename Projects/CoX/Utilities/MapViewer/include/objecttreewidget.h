#pragma once

#include <QDockWidget>
#include <QAbstractItemModel>
namespace Ui {
    class ObjectTreeWidget;
}

class ObjectTreeWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit ObjectTreeWidget(QWidget *parent = 0);
    ~ObjectTreeWidget();
    void set_model(QAbstractItemModel *model);
private:
    Ui::ObjectTreeWidget *ui;
};

#endif // OBJECTTREEWIDGET_H
