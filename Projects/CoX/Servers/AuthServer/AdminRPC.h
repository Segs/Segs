#pragma once

#include <QObject>
#include <QVariant>

class AdminRPC : public QObject
{
    Q_OBJECT
    friend void startWebSocketServer(const char *addr,int port);
    class AuthHandler *m_auth_handler;
private:
    AdminRPC(); // restrict construction to startWebSocketServer
public:

    Q_INVOKABLE bool heyServer();
    Q_INVOKABLE QString helloServer();
    Q_INVOKABLE QString getVersion();
};

void startWebSocketServer(const char *addr="127.0.0.1",int port=6001);
