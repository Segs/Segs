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


#include "NetworkManager.h"
#include "Globals.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QJsonArray>
#include <QFile>

NetworkManager::NetworkManager()
{

}

// Get and compare latest releases - GitHub API

void NetworkManager::get_latest_releases()
{

    all_releases_networkManager = new QNetworkAccessManager(this);
    QUrl url("https://api.github.com/repos/segs/segs/releases");
    request.setUrl(url);
    m_network_reply = all_releases_networkManager->get(request);
    connect(all_releases_networkManager,&QNetworkAccessManager::finished,this,&NetworkManager::check_reply);

}

void NetworkManager::check_reply()
{
    if(m_network_reply->error() == QNetworkReply::NoError)
    {
        qDebug()<<"No Network Error";
        store_latest_releases();
    }
    else
    {
        qDebug()<<m_network_reply->error();
        qDebug()<<"Network Error";
        emit releasesReadyToRead(m_network_reply->errorString());
    }
}

// TODO: Error handling at some stage
void NetworkManager::store_latest_releases()
{
    // Read json reply and push into QVector
    QByteArray reply = m_network_reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reply);
    QJsonArray json_array = jsonResponse.array();

    for(const QJsonValue &value : json_array)
    {
        QJsonObject json_obj = value.toObject();
        SegsReleaseInfo tmp_release_info{json_obj["tag_name"].toString(),json_obj["name"].toString(),json_obj["id"].toString()
                    ,json_obj["body"].toString(),json_obj["html_url"].toString()};
        g_segs_release_info.push_back(tmp_release_info);
    }
    qDebug()<<"Api retrieval and stored success";
    qDebug()<<g_segs_release_info[0].github_url;
    emit releasesReadyToRead();
}


//!@}
