/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "UpgradeHooks.h"

#include "Logging.h"

#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

namespace {

struct UpgradeHook
{
    TableSchema m_table_schema;
    std::function<void(const DatabaseConfig &)> m_handler;
};

// handlers
void upgradeHandler_001(const DatabaseConfig &cfg);

static const UpgradeHook g_defined_upgrade_hooks[] = {
    /* Table Schema Changes */
    {{"segs_game", "db_version", 10, "2018-11-21 22:56:43"}, upgradeHandler_001},
};


/*
 * upgradeHandlers
 */
void upgradeHandler_001(const DatabaseConfig &cfg)
{
    qWarning() << "DO UPGRADE FOR 001";
}


/*
 * add new handlers above this comment
 */

} // end anonymouse namespace


/*
 * runUpgrade for executing upgrade handlers
 */
void runUpgrade(const DatabaseConfig &cfg, const TableSchema &current)
{
    for (const UpgradeHook &hook : g_defined_upgrade_hooks)
    {
        if(hook.m_table_schema == current)
        {
            hook.m_handler(cfg);
            return; // return here to avoid unknown hook msg
        }
    }
    qWarning() << "Unknown db upgrade hook:" << current.m_db_name << current.m_table_name << current.m_version;
}
