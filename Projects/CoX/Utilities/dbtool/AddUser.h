/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "DatabaseConfig.h"
#include "CreateDB.h"

#include <inttypes.h>

dbToolResult addAccount(const DatabaseConfig &char_database, const QString & username,
                const QString & password, uint16_t access_level);
