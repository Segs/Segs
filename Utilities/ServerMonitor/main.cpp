/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup ServerMonitor Utilities/ServerMonitor
 * @{
 */

#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("Segs");
    a.setApplicationName("SegsMonitor");
    MainWindow w;
    w.show();

    return a.exec();
}

//! @}
