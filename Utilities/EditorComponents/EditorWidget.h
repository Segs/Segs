#pragma once

#include <QtWidgets/QWidget>

class EditorWidget : public QWidget
{
    Q_OBJECT
    bool modified = false;
public:
    explicit EditorWidget(QWidget *parent = nullptr);

signals:

};

