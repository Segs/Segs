/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2022 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <QFile>
#include <QObject>

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    explicit ThemeManager(QObject *parent = nullptr);

public slots:
    void setLightTheme();
    void setDarkTheme();

private:
    void startup();
    void setTheme(QFile &theme);
    void setUserPref(QString theme);
    QString getUserPref();


signals:

};
