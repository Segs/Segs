/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */


#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    NetworkManager();

public slots:
    void get_latest_releases();
    void store_latest_releases();
    void check_reply();

signals:
    void releasesReadyToRead(const QString &error = "No Errors");

private:
    QNetworkReply *m_network_reply;
    QNetworkAccessManager *all_releases_networkManager;
    QNetworkRequest request;
};

#endif // NETWORKMANAGER_H
