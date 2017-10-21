/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#pragma once

#include <QtSql/QSqlDatabase>
class DbTransactionGuard
{
    QSqlDatabase &m_db;
    bool proper_commit;
public:
    DbTransactionGuard(QSqlDatabase &db) : m_db(db),proper_commit(false)
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
