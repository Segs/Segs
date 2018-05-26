/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup SEGSAdmin Projects/CoX/Utilities/SEGSAdmin
 * @{
 */

#include "SEGSAdminTool.h"
#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SEGSAdminTool w;
    w.show();

    // Add custom fonts
    QFontDatabase fontDB;
    fontDB.addApplicationFont(":/fonts/dejavusanscondensed.ttf");

    return a.exec();
}

//!@}
