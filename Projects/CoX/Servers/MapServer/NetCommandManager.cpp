/*
* Super Entity Game Server Project
* http://segs.sf.net/
* Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
* This software is licensed! (See License.txt for details)
*

*/

#include "NetCommandManager.h"
#include "MapClient.h"
static void FillCommands()
{
    NetCommandManager *cmd_manager = NetCommandManagerSingleton::instance();
    NetCommand::Argument arg1={1,NULL};
    NetCommand::Argument arg_1float={3,NULL};
    vector<NetCommand::Argument> args;
    args.push_back(arg1);
    vector<NetCommand::Argument> fargs;
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

    cmd_manager->addCommand(new NetCommand(3,"time",fargs)); // unknown = 12
    cmd_manager->addCommand(new NetCommand(3,"timescale",fargs)); // unknown = 13
    cmd_manager->addCommand(new NetCommand(3,"timestepscale",fargs)); // unknown = 14
    cmd_manager->addCommand(new NetCommand(3,"pause",args));
    cmd_manager->addCommand(new NetCommand(3,"disablegurneys",args));
//    cmd_manager->addCommand(new NetCommand(9,"nodynamiccollisions",args));
//    cmd_manager->addCommand(new NetCommand(9,"noentcollisions",args));
//    cmd_manager->addCommand(new NetCommand(9,"pvpmap",args)); // unknown 16
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
                ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("CommRecv %s:arg%d : %d\n"),m_name.c_str(),i,res));
                break;
            }
            case 2:
            case 4:
            {
                std::string res;
                bs.GetString(res); // postprocessed
                ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("CommRecv %s:arg%d : %s\n"),m_name.c_str(),i,res.c_str()));
                break;
            }
            case 3:
            {
                float res = bs.GetFloat();
                ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("CommRecv %s:arg%d : %f\n"),m_name.c_str(),i,res));
                break;
            }
            case 5:
            {
                float res1 = normalizedCircumferenceToFloat(bs.GetBits(14),14);
                ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("CommRecv %s:arg%d : %s\n"),m_name.c_str(),i,res1));
                break;
            }
            case 6:
                break;
            case 7:
            {
                float res1 = bs.GetFloat();
                float res2 = bs.GetFloat();
                float res3 = bs.GetFloat();
                ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("CommRecv %s:arg%d : %f,%f,%f\n"),m_name.c_str(),i,res1,res2,res3));
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

NetCommand * NetCommandManager::getCommandByName( const std::string &name )
{
    return m_name_to_command[name];
}
void NetCommandManager::serializeto(BitStream &tgt, const vNetCommand &commands, const vNetCommand &commands2 )
{
    if(commands.size()==0)
    {
        tgt.StorePackedBits(1,~0);//0xFFFFFFFF
    }
    else
    {
        for(uint32_t i=0; i<(uint32_t)commands.size(); i++)
        {
            tgt.StorePackedBits(1,i+1);
            tgt.StoreString(commands[i]->m_name);
        }
    }
    tgt.StorePackedBits(1,(uint32_t)commands2.size());
    if(commands2.size()>0)
    {
        for(uint32_t i=0; i<(uint32_t)commands2.size(); i++)
        {
            tgt.StoreString(commands2[i]->m_name);
        }
    }
}
void NetCommandManager::SendCommandShortcuts( MapClient *client,BitStream &tgt,const vector<NetCommand *> &commands2 )
{
    static bool initialized=false;
    if(!initialized)  {
        initialized=true;
        FillCommands();
    }

    switch(client->account_info().access_level())
    {
        case 0:
        case 1:
            // add shortcuts to client's shortcut map.
            for(size_t i=0; i<m_commands_level0.size(); ++i)
                client->AddShortcut(i,m_commands_level0[i]);
            serializeto(tgt,m_commands_level0,commands2);
            break;
        default:
            assert(false);
    }
}
