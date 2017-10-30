/*
* Super Entity Game Server
* http://segs.sf.net/
* Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
* This software is licensed! (See License.txt for details)
*
*/

#define VersionString "segs v0.4.0 alpha (The Real Beginning)";
#define CopyrightString "Super Entity Game Server\nhttp://segs.sf.net/\nCopyright (c) 2006-2017 Super Entity Game Server Team (see Authors.txt)\nThis software is licensed! (See License.txt for details)\n";
//const char *AdminVersionString="Undefined";
//const char *AuthVersionString="Undefined";
//const char *GameVersionString="Undefined";
//const char *MapVersionString="Undefined";

// Contains version information for the various server modules
class VersionInfo
{
public:
    static const char *getAdminVersion(void);
    static const char *getAuthVersion(void)
{
        return VersionString;
}

    static const char *getGameVersion(void);
    static const char *getMapVersion(void);
    static const char *getCopyright(void)
{
        return CopyrightString;
}
};

#undef VersionString
#undef CopyrightString

