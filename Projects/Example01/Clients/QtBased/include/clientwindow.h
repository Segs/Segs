/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once

#include <QMainWindow>

namespace Ui
{
    class ClientWindow;
}

class ClientWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit ClientWindow(QWidget *parent = 0);
        ~ClientWindow();

    private:
        Ui::ClientWindow *ui;
};
