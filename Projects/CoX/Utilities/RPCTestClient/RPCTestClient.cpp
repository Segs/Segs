/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup RPCTestClient Projects/CoX/Utilities/RPCTestClient
 * @{
 */

#include "RPCTestClient.h"
#include "ui_RPCTestClient.h"
#include "Servers/AuthServer/AdminRPC.h"

#include "jcon/json_rpc_tcp_client.h"

#include <QDebug>
#include <QVariantList>

RPCTestClient::RPCTestClient(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RPCTestClient)
{
    ui->setupUi(this);
    connect(ui->request_button,&QPushButton::clicked,this,&RPCTestClient::call_rpc);
    populate();
}

RPCTestClient::~RPCTestClient()
{
    delete ui;
}

void RPCTestClient::populate()
{

    QStringList types = {
        "int",
        "string"
    };
    m_type_combo_boxes = ui->centralwidget->findChildren<QComboBox *>();
    for (int i = 0; i < m_type_combo_boxes.size(); ++i)
    {
        m_type_combo_boxes.at(i)->addItems(types);
    }
}

void RPCTestClient::call_rpc()
{
    QVariantList arg_list;
    QList<QLineEdit*> arg_line_edits;
    arg_line_edits << ui->centralwidget->findChildren<QLineEdit *>("arg_1")
                   << ui->centralwidget->findChildren<QLineEdit *>("arg_2")
                   << ui->centralwidget->findChildren<QLineEdit *>("arg_3");

    qDebug()<<arg_line_edits.size();
    for (int i = 0; i < arg_line_edits.size(); ++i)
    {
        if (!arg_line_edits.at(i)->text().isEmpty())
        {
            if (m_type_combo_boxes.at(i)->currentText() == "int")
            {
                int arg = arg_line_edits.at(i)->text().toInt();
                arg_list.push_back(arg);
            }
            else
            {
                QString arg = arg_line_edits.at(i)->text();
                arg_list.push_back(arg);
            }
        }
    }

    QString auth_addr = ui->server_ip->text();
    QString service = ui->rpc_service->text();
    qDebug()<<"RPC Client connecting to: " << auth_addr;
    auto rpc_client = new jcon::JsonRpcTcpClient(this);
    if(rpc_client->connectToServer(auth_addr, 6001))
    {
        qDebug()<<"Connected to RPC Server";

        if (arg_list.isEmpty())
        {
            auto result = rpc_client->call(service);
            qDebug() << service + " called";
            if (result->isSuccess())
            {
                QVariant res = result->result();
                ui->response->appendPlainText("Response: " + res.toString());
            }
            else
            {
                QString err_str = result->toString();
                qDebug() << "Result: " << err_str;
            }
        }
        else
        {
            auto result = rpc_client->callExpandArgs(service, arg_list);
            qDebug() << service + " called with args " << arg_list;
            if (result->isSuccess())
            {
                QVariant res = result->result();
                ui->response->appendPlainText("Response: " + res.toString());
            }
            else
            {
                QString err_str = result->toString();
                qDebug() << "Result: " << err_str;
            }
        }
    }
    else
    {
        qDebug() << "Failed to connect";
    }
}

//!@}
