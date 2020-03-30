/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
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
