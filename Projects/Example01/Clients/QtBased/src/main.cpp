/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup ExampleQtAuthClient Projects/Example01/Clients/QtBased
 * @{
 */

#include <QtGui/QApplication>
#include "clientwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClientWindow w;
    w.show();

    return a.exec();
}

//! @}
