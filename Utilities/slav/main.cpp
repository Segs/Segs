/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup slav Utilities/slav
 * @{
 */

#include <QtGui/QSurfaceFormat>
#include <QtWidgets/QApplication>
#include "SLAVLogic.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    SLAVLogic app_logic;
    app_logic.start();
    app.exec();
    return 0;
}

//! @}
