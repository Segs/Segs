/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

// ACE includes

#include "Servers/HandlerLocator.h"
#include "Components/EventProcessor.h"

#include <ace/INET_Addr.h>
#include <ace/Synch.h>

#include <QObject>
#include <QVariant>
#include <QtCore/QDateTime>

namespace SEGSEvents
{
struct CreateAccountMessage;
struct AuthDbStatusMessage;
}

enum class SocketType
{
    tcp,
    websocket
};

class AdminRPC : public QObject, public EventProcessor
{
    Q_OBJECT
    class AuthHandler *m_auth_handler;
    friend void startRPCServer();

private:
    AdminRPC(); // restrict construction to startRPCServer
    ~AdminRPC() override;

public:
    Q_INVOKABLE bool heyServer();
    Q_INVOKABLE QString helloServer();
    Q_INVOKABLE QString getVersion();
    Q_INVOKABLE QString getVersionName();
    Q_INVOKABLE QString getStartTime();
    Q_INVOKABLE QVariantMap getWebUIData(QString const& version);
    Q_INVOKABLE QString ping();
    Q_INVOKABLE QString addUser(const QString &username, const QString &password, int access_level);

protected:
    ACE_INET_Addr                       m_location;     //!< address rpc server will bind at.
    ACE_Thread_Mutex                    m_mutex;        //!< used to prevent multiple threads accessing config reload function
    bool                                ReadConfig();
    void                                SetStartTime();
    QString                             m_start_time;
    SocketType                          m_socket_type;
    static uint64_t                     s_last_token;
    void                                on_db_error(SEGSEvents::AuthDbStatusMessage *ev);
    QMap<int, QString>                  m_completion_state;

signals:
    void responseRecieved();


    // EventProcessor interface
public:
    IMPL_ID(AdminRPC)

protected:
    void dispatch(SEGSEvents::Event *ev) override;
    void serialize_from(std::istream &is) override;
    void serialize_to(std::ostream &is) override;
};

void startRPCServer();
