/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
// ACE includes
#include <ace/ACE.h>
#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/Synch.h>

#include <list>
#include <string>
#include <memory>

class CharacterDatabase;
class QString;
//! The AdminServer class handles administrative functions such as account saving, account banning, etcetera.
class _AdminServer
{
public:

    // Constructor/Destructor
                                _AdminServer(void);
                                ~_AdminServer(void);

    //bool AccountBlocked(const char *login) const; // Check if account is blocked.

    // Internal Admin server interface
    CharacterDatabase *         character_db(){return m_char_db.get();}

    bool ReadConfig();
    bool Run();
    bool ShutDown(const QString &reason);
protected:
    std::unique_ptr<CharacterDatabase> m_char_db;      //!< character database access object
};
typedef ACE_Singleton<_AdminServer,ACE_Thread_Mutex> AdminServer; // AdminServer Interface
