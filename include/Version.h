/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#ifndef SEGS_VERSION
#define SEGS_VERSION "0.7.0" // set in cmake, but defined here if not available
#endif
#ifndef SEGS_BRANCH
#define SEGS_BRANCH "unknown"
#endif
#ifndef SEGS_DESCRIPTION
#define SEGS_DESCRIPTION "unknown"
#endif

#define ProjectName "SEGS"
#define VersionName "Awakening"
#define VersionString ProjectName " v" SEGS_VERSION "-" SEGS_BRANCH " (" VersionName ") [" SEGS_DESCRIPTION "]"
#define CopyrightString "Super Entity Game Server\nhttp://github.com/Segs/\nCopyright (c) 2006-2019 Super Entity Game Server Team (see AUTHORS.md)\nThis software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.\n";

static constexpr int s_config_version = 2;
static constexpr int s_auth_db_version = 1;
static constexpr int s_game_db_version = 10;

// Contains version information for the various server modules
class VersionInfo
{
public:
    static const char *getAuthVersion(void) { return VersionString; }
    static const char *getAuthVersionNumber(void) { return SEGS_VERSION; }
    static const char *getVersionName(void) { return VersionName; }
    static const char *getCopyright(void){ return CopyrightString; }
    static constexpr int getConfigVersion(void) { return s_config_version; }
    static constexpr int getRequiredAuthDBVersion(void) { return s_auth_db_version; }
    static constexpr int getRequiredGameDBVersion(void) { return s_game_db_version; }
};

#undef ProjectName
#undef VersionName
#undef VersionString
#undef CopyrightString
#undef SEGS_VERSION
#undef SEGS_BRANCH
#undef SEGS_DESCRIPTION

