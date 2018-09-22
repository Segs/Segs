#pragma once

#include <QObject>

class AdminRPC : public QObject
{
    Q_OBJECT
    friend void startWebSocketServer(const char *addr,int port);
private:
    AdminRPC(); // restrict construction to startWebSocketServer
public:

    Q_INVOKABLE bool heyServer();
};

void startWebSocketServer(const char *addr="127.0.0.1",int port=6001);
