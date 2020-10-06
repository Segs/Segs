/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Components/BitStream.h"

#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <map>
#include <vector>

struct MapClientSession;

class NetCommand
{
    float normalizedCircumferenceToFloat(int number,int numbits)
    {
        // something like this : ((number*3.141592)/(1<<numbits))-3.141592
        float f=((float(number)*3.141592f)/(1<<numbits))-3.141592f;
        return f;
    }
public:

    struct Argument
    {
        int type;
        void *targetvar;
    };
    NetCommand(int acl,const QString &name,std::vector<Argument> &args):m_arguments(args)
    {
        m_required_access_level=acl;
        m_name=name;
    }
    int serializefrom(BitStream &bs);
    int clientside_idx;
    int m_required_access_level;
    QString m_name;
    std::vector<Argument> m_arguments;

};

class NetCommandManager
{
using   vNetCommand = std::vector<NetCommand *>;

        QHash<QString, NetCommand *> m_name_to_command;
        vNetCommand                  m_commands_level0;
        void                         serializeto(BitStream &tgt, const vNetCommand &commands);

public:
        void        UpdateCommandShortcuts(MapClientSession *client, std::vector<QString> &commands);
        NetCommand *getCommandByName(const QString &name);
        void        addCommand(NetCommand *cmd);
};
typedef ACE_Singleton<NetCommandManager,ACE_Thread_Mutex> NetCommandManagerSingleton; // AdminServer Interface
