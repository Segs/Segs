#ifndef OBJECTTREEWIDGET_H
#define OBJECTTREEWIDGET_H

#include <QDockWidget>

namespace Ui {
    class ObjectTreeWidget;
}

class ObjectTreeWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit ObjectTreeWidget(QWidget *parent = 0);
    ~ObjectTreeWidget();

private:
    Ui::ObjectTreeWidget *ui;
};

#endif // OBJECTTREEWIDGET_H
