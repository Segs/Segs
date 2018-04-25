/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once

#include <QtSql/QSqlDatabase>
class DbTransactionGuard
{
    QSqlDatabase &m_db;
    bool proper_commit = false;
public:
    DbTransactionGuard(QSqlDatabase &db) : m_db(db)
    {
        m_db.transaction();
    }
    void commit() {proper_commit=true;}
    ~DbTransactionGuard()
    {
        if(proper_commit)
            m_db.commit();
        else
            m_db.rollback();
    }

};
