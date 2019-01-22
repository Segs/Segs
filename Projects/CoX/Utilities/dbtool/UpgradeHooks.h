/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "UpgradeDB.h"

struct UpgradeHook
{
    TableSchema m_table_schema;
    std::function<bool(const DatabaseConfig &)> m_handler;
};

extern const std::vector<UpgradeHook> g_segs_upgrade_hooks;
