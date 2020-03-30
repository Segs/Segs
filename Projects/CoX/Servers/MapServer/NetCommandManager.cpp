/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "NetCommandManager.h"
#include "MapClientSession.h"

#include <vector>

static void FillCommands()
{
    NetCommandManager *cmd_manager = NetCommandManagerSingleton::instance();
    NetCommand::Argument arg1={1,nullptr};
    NetCommand::Argument arg_1float={3,nullptr};
    std::vector<NetCommand::Argument> args;
    args.push_back(arg1);
    std::vector<NetCommand::Argument> fargs;
    fargs.push_back(arg_1float);
//    cmd_manager->addCommand(new NetCommand(9,"controldebug",args));
//    cmd_manager->addCommand(new NetCommand(9,"nostrafe",args));
//    cmd_manager->addCommand(new NetCommand(9,"alwaysmobile",args));
//    cmd_manager->addCommand(new NetCommand(9,"repredict",args));
//    cmd_manager->addCommand(new NetCommand(9,"neterrorcorrection",args));
//    cmd_manager->addCommand(new NetCommand(9,"speed_scale",fargs));
//    cmd_manager->addCommand(new NetCommand(9,"svr_lag",args));
//    cmd_manager->addCommand(new NetCommand(9,"svr_lag_vary",args));
//    cmd_manager->addCommand(new NetCommand(9,"svr_pl",args));
//    cmd_manager->addCommand(new NetCommand(9,"svr_oo_packets",args));
//    cmd_manager->addCommand(new NetCommand(9,"client_pos_id",args));
//    cmd_manager->addCommand(new NetCommand(9,"atest0",args));
//    cmd_manager->addCommand(new NetCommand(9,"atest1",args));
//    cmd_manager->addCommand(new NetCommand(9,"atest2",args));
//    cmd_manager->addCommand(new NetCommand(9,"atest3",args));
//    cmd_manager->addCommand(new NetCommand(9,"atest4",args));
//    cmd_manager->addCommand(new NetCommand(9,"atest5",args));
//    cmd_manager->addCommand(new NetCommand(9,"atest6",args));
//    cmd_manager->addCommand(new NetCommand(9,"atest7",args));
//    cmd_manager->addCommand(new NetCommand(9,"atest8",args));
//    cmd_manager->addCommand(new NetCommand(9,"atest9",args));
//    cmd_manager->addCommand(new NetCommand(9,"predict",args));
//    cmd_manager->addCommand(new NetCommand(9,"notimeout",args)); // unknown-10,argtype-1
//    cmd_manager->addCommand(new NetCommand(9,"selected_ent_server_index",args));
//    cmd_manager->addCommand(new NetCommand(9,"record_motion",args));

    cmd_manager->addCommand(new NetCommand(3,"time",fargs));
    cmd_manager->addCommand(new NetCommand(3,"timescale",fargs));
    cmd_manager->addCommand(new NetCommand(3,"timestepscale",fargs));
    cmd_manager->addCommand(new NetCommand(3,"pause",args));
    cmd_manager->addCommand(new NetCommand(3,"disablegurneys",args));
//    cmd_manager->addCommand(new NetCommand(9,"nodynamiccollisions",args));
//    cmd_manager->addCommand(new NetCommand(9,"noentcollisions",args));
//    cmd_manager->addCommand(new NetCommand(9,"pvpmap",args));
}

int NetCommand::serializefrom( BitStream &bs )
{
    for(size_t i=0; i<m_arguments.size(); i++)
    {
        switch(m_arguments[i].type)
        {
            case 1:
            {
                int res=bs.GetPackedBits(1);
                if(m_arguments[i].targetvar)
                    *((int *)m_arguments[i].targetvar) = res;
                qDebug("CommRecv %s:arg%zu : %d", qPrintable(m_name),i,res);
                break;
            }
            case 2:
            case 4:
            {
                QString res;
                bs.GetString(res); // postprocessed
                qDebug("CommRecv %s:arg%zu : %s", qPrintable(m_name),i,qPrintable(res));
                break;
            }
            case 3:
            {
                float res = bs.GetFloat();
                qDebug("CommRecv %s:arg%zu : %f", qPrintable(m_name),i,res);
                break;
            }
            case 5:
            {
                float res1 = normalizedCircumferenceToFloat(bs.GetBits(14),14);
                qDebug("CommRecv %s:arg%zu : %f", qPrintable(m_name),i,res1);
                break;
            }
            case 6:
                break;
            case 7:
            {
                float res1 = bs.GetFloat();
                float res2 = bs.GetFloat();
                float res3 = bs.GetFloat();
                qDebug("CommRecv %s:arg%zu : %f,%f,%f", qPrintable(m_name),i,res1,res2,res3);
                break;
            }
        }
    }
    return 1;
}

void NetCommandManager::addCommand( NetCommand *cmd )
{
    assert(m_name_to_command.find(cmd->m_name)==m_name_to_command.end());
    m_name_to_command[cmd->m_name]=cmd;
    m_commands_level0.push_back(cmd);
}

NetCommand * NetCommandManager::getCommandByName( const QString &name )
{
    return m_name_to_command[name];
}

void NetCommandManager::serializeto(BitStream &tgt, const vNetCommand &commands)
{
    for(uint32_t i=0; i<commands.size(); i++)
    {
        tgt.StorePackedBits(1,i+1);
        tgt.StoreString(commands[i]->m_name);
    }
    tgt.StorePackedBits(1,~0u); // end of command list
}
void NetCommandManager::UpdateCommandShortcuts(MapClientSession *client, std::vector<QString> &commands)
{
    static bool initialized=false;
    if(!initialized)  {
        initialized=true;
        FillCommands();
    }
    // add shortcuts to client's shortcut map.
    //TODO: differentiate the commands based on access level ?
    commands.reserve(m_commands_level0.size());
    for(size_t i=0, total = m_commands_level0.size(); i<total; ++i)
    {
        client->AddShortcut(i,m_commands_level0[i]);
        commands.push_back(m_commands_level0[i]->m_name);
    }
}
//! @}
