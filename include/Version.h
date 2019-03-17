/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#define ProjectName "SEGS"
#define VersionNumber SEGS_VERSION // from cmake and git revision
#define VersionName "Outbreak"
#define VersionString ProjectName " v" VersionNumber " (" VersionName ") [" SEGS_BRANCH ": " SEGS_REVISION "]"
#define CopyrightString "Super Entity Game Server\nhttp://github.com/Segs/\nCopyright (c) 2006-2019 Super Entity Game Server Team (see AUTHORS.md)\nThis software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.\n";

static constexpr int s_config_version = 1;
static constexpr int s_auth_db_version = 1;
static constexpr int s_game_db_version = 9;

// Contains version information for the various server modules
class VersionInfo
{
public:
    static const char *getAuthVersion(void) { return VersionString; }
    static const char *getAuthVersionNumber(void) { return VersionNumber;}
    static const char *getVersionName(void) { return VersionName; }
    static const char *getCopyright(void){ return CopyrightString; }
    static constexpr int getConfigVersion(void) { return s_config_version; }
    static constexpr int getRequiredAuthDBVersion(void) { return s_auth_db_version; }
    static constexpr int getRequiredGameDBVersion(void) { return s_game_db_version; }
};

#undef ProjectName
#undef VersionName
#undef VersionNumber
#undef VersionString
#undef CopyrightString

