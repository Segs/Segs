/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
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
