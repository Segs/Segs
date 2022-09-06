/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup SEGSAdmin Projects/CoX/Utilities/SEGSAdmin
 * @{
 */

#include "SEGSAdminTool.h"
#include "Globals.h"
#include "Version.h"
#include <QApplication>
#include <QFontDatabase>
#include <QSplashScreen>

int main(int argc, char *argv[])
{
    static const char ENV_VAR_QT_DEVICE_PIXEL_RATIO[] = "QT_DEVICE_PIXEL_RATIO";
    if(!qEnvironmentVariableIsSet(ENV_VAR_QT_DEVICE_PIXEL_RATIO)
        && !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR")
        && !qEnvironmentVariableIsSet("QT_SCALE_FACTOR")
        && !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS")) {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    }
    
    QApplication a(argc, argv);

    // Add custom fonts
    QFontDatabase fontDB;
    fontDB.addApplicationFont(":/fonts/dejavusanscondensed.ttf");
    QPixmap pixmap(":/logo/Resources/splash-lg.png");
    QFont dejavu_font_splash;
    dejavu_font_splash.setFamily("DejaVu Sans Condensed");
    dejavu_font_splash.setPointSize(12);
    dejavu_font_splash.setBold(true);
    dejavu_font_splash.setStretch(125);

    // Splash screen
    QSplashScreen *splash = new QSplashScreen(pixmap);
    splash->setFont(dejavu_font_splash);
    splash->show();
    splash->showMessage(QString("Loading ") + VersionInfo::getAuthVersionNumber() + " " + VersionInfo::getVersionName(),
                        Qt::AlignRight | Qt::AlignBottom, Qt::yellow);
    SEGSAdminTool w;
    splash->finish(&w);
    w.show();
    return a.exec();
}

//!@}
