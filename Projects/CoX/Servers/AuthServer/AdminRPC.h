/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

// ACE includes
#include <ace/INET_Addr.h>
#include <ace/Synch.h>

#include <QObject>
#include <QVariant>
#include <QtCore/QDateTime>

class AdminRPC : public QObject
{
    Q_OBJECT
    class AuthHandler *m_auth_handler;
    friend void startWebSocketServer();
private:
    AdminRPC(); // restrict construction to startWebSocketServer
public:
    Q_INVOKABLE bool heyServer();
    Q_INVOKABLE QString helloServer();
    Q_INVOKABLE QString getVersion();
    Q_INVOKABLE QString getStartTime();
    Q_INVOKABLE QString ping();
protected:
    ACE_INET_Addr                       m_location;     //!< address websockets will bind at.
    ACE_Thread_Mutex                    m_mutex;        //!< used to prevent multiple threads accessing config reload function
    bool                                ReadConfig();
    void 								SetStartTime();
    QString                             m_start_time;
};

void startWebSocketServer();
