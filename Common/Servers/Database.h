/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
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
