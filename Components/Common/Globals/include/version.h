/*
* Super Entity Game Server
* http://segs.sf.net/
* Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
* This software is licensed! (See License.txt for details)
*
*/

const char *VersionString="\nsegs v0.2.0 (The People!)\n";
const char *AdminVersionString="Undefined";
const char *AuthVersionString="Undefined";
const char *GameVersionString="Undefined";
const char *MapVersionString="Undefined";
const char *CopyrightString="\nSuper Entity Game Server\nhttp://segs.sf.net/\nCopyright (c) 2006 Super Entity Game Server Team (see Authors.txt)\nThis software is licensed! (See License.txt for details)\n\n";

// Contains version information for the various server modules
class VersionInfo
{
    // Con/Destructors
public:
    VersionInfo();
    ~VersionInfo();

    char getAdminVersion(void) const;
    char getAuthVersion(void) const;
    char getGameVersion(void) const;
    char getMapVersion(void) const;
    char getCopyright(void) const;
};

VersionInfo::VersionInfo()
{
}

VersionInfo::~VersionInfo()
{
}

char VersionInfo::getAuthVersion(void) const
{
    ACE_DEBUG ((LM_INFO, VersionString));   //Will print to console and file
    return 0;
}

char VersionInfo::getCopyright(void) const
{
    ACE_DEBUG ((LM_INFO, CopyrightString));   //Will print to console and file
    return 0;
}
