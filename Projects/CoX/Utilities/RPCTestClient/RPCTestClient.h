/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <QMainWindow>
#include <QComboBox>

namespace Ui {
class RPCTestClient;
}

class RPCTestClient : public QMainWindow
{
    Q_OBJECT
    class JsonRpcTcpClient *m_json_rpc;

public:
    explicit RPCTestClient(QWidget *parent = nullptr);
    ~RPCTestClient();

public slots:
    void call_rpc();

private:
    Ui::RPCTestClient *ui;
    QList<QComboBox*> m_type_combo_boxes;
    void populate();
};
