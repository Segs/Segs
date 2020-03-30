/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <QObject>
#include <unordered_map>

class QNetworkAccessManager;
class QNetworkReply;
class QIODevice;
struct ProjectApp;
struct ProjectList;
class AppVersionManifest;

struct OperationData
{
    QNetworkReply * reply;
    QString requested_url;
    QIODevice *target;
    int targetType;
};
class ServerConnection : public QObject
{
    Q_OBJECT
    QString m_base_url;
    std::map< QNetworkReply *,OperationData> m_operations_in_flight;
    QNetworkAccessManager *m_network_manager;
public:
    explicit ServerConnection(const QString &baseUrl,QObject *parent = 0);

signals:
    void downloadProgress(const QString &req_url,qint64 bytesReceived, qint64 bytesTotal);
    void networkFailure(const QString &msg);

    void retrievedManifest(QString req_url,QString manifestData);
    void appDataAvailable(ProjectApp *app_data);
    void projectListAvailable(ProjectList *projects);
    void manifestAvailable(AppVersionManifest *projects);
private slots:
    void httpDownloadFinished(QNetworkReply *);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

    // void onAppDataRequested(QString req_url)
public slots:
    void onRequestProjectList(); // should be called periodically to refresh the server list ( 1/minute ? )
    void onRequestAppData();     // called at app startup to download slav.app_data
    void onRequestManifest(const QString &manifestname);
    void onRequestFileList(const QString &base_path,const QStringList &files);
};

#endif // SERVERCONNECTION_H
