/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup SEGSAdmin Projects/CoX/Utilities/SEGSAdmin
 * @{
 */

#include "SEGSAdminTool.h"
#include <QApplication>
#include <QFontDatabase>
#include <QSplashScreen>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Add custom fonts
    QFontDatabase fontDB;
    fontDB.addApplicationFont(":/fonts/dejavusanscondensed.ttf");
    QPixmap pixmap(":/logo/segs_splash_1.png");
    QFont dejavu_font_splash;
    dejavu_font_splash.setFamily("DejaVu Sans Condensed");
    dejavu_font_splash.setPointSize(12);
    dejavu_font_splash.setBold(true);
    dejavu_font_splash.setStretch(125);

    // Splash screen
    QSplashScreen *splash = new QSplashScreen(pixmap);
    splash->setFont(dejavu_font_splash);
    splash->show();
    splash->showMessage("Loading", Qt::AlignBottom, Qt::red);

    SEGSAdminTool w;
    splash->finish(&w);
    w.show();
    return a.exec();
}

//!@}
