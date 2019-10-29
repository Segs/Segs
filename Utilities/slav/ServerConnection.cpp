/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup slav Utilities/slav
 * @{
 */

#include "ServerConnection.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QBuffer>
#include <QtCore/QDebug>
#include <cassert>

ServerConnection::ServerConnection(const QString & baseUrl, QObject *parent) : QObject(parent), m_base_url(baseUrl)
{
    m_network_manager = new QNetworkAccessManager(this);
    connect(m_network_manager, &QNetworkAccessManager::finished,this,&ServerConnection::httpDownloadFinished);
}

void ServerConnection::onRequestManifest(const QString &manifestname) {
    QString request_url = m_base_url+"/"+manifestname+".manifest";
    QNetworkRequest request_ob(request_url);
    request_ob.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);
    QNetworkReply *reply = m_network_manager->get(request_ob);
    connect(reply, &QNetworkReply::downloadProgress, this, &ServerConnection::onDownloadProgress);
    m_operations_in_flight.emplace(reply,OperationData {reply,request_url,new QBuffer,0});
}

void ServerConnection::onRequestFileList(const QString & base_path, const QStringList & files)
{
    Q_UNUSED(base_path);
    Q_UNUSED(files);
    assert(false);
}

void ServerConnection::httpDownloadFinished(QNetworkReply *reply)
{
    auto op_iter = m_operations_in_flight.find(reply);
    assert(op_iter != m_operations_in_flight.end());
    if(reply->error()) {
        emit networkFailure(tr("Download failure: %1").arg(reply->errorString()));
        m_operations_in_flight.erase(reply);
        reply->deleteLater();
        return;
    }
    const QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    OperationData  op_data(op_iter->second);
    m_operations_in_flight.erase(reply);

    if(!redirectionTarget.isNull()) {
        qCritical() << "Redirections are not handled yet";
    }
    op_data.target->write(reply->readAll());
    op_data.target->close();
    switch (op_data.targetType) {
        case 0: // a manifest file
        {
            QBuffer *buf(qobject_cast<QBuffer *>(op_data.target));
            emit retrievedManifest(op_data.requested_url,buf->data());
            delete buf;
            op_data.target = nullptr;
            break;
        }
        default:
            assert(false);
    }
    reply->deleteLater();
}

void ServerConnection::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    QNetworkReply * repl = qobject_cast<QNetworkReply *>(sender());
    auto iter = m_operations_in_flight.find(repl);
    assert(iter!=m_operations_in_flight.end());
    emit downloadProgress(iter->second.requested_url,bytesReceived,bytesTotal);
}

void ServerConnection::onRequestProjectList()
{

}

void ServerConnection::onRequestAppData()
{

}

//! @}
