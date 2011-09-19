/*
 * Super Entity Game Server Project 
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */
#pragma once
#include <vector>
#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>
#include "BitStream.h"

class MapClient;
class NetCommand
{
	float normalizedCircumferenceToFloat(int number,int numbits)
	{
		// something like this : ((number*3.141592)/(1<<numbits))-3.141592
        float f=((float(number)*3.141592f)/(1<<numbits))-3.141592f;
        f=0.0f; // since i'm not sure about correctness of the above
		return f;
	}
public:

	struct Argument
	{
		int type;
		void *targetvar;
	};
	NetCommand(int acl,const std::string &name,std::vector<Argument> &args):m_arguments(args)
	{
		m_required_access_level=acl;
		m_name=name;
	}
	int serializefrom(BitStream &bs);
	int clientside_idx;
	int m_required_access_level;
	std::string m_name;
	std::vector<Argument> m_arguments;

};
class NetCommandManager
{
	std::map<std::string,NetCommand *> m_name_to_command;
	std::vector<NetCommand *> m_commands_level0;
	void            SendCommandShortcutsWorker(MapClient *client,
											   BitStream &tgt,
											   const std::vector<NetCommand *> &commands,
											   const std::vector<NetCommand *> &commands2);
public:
	void            SendCommandShortcuts(MapClient *client,
										 BitStream &tgt,
										 const std::vector<NetCommand *> &commands2);
	NetCommand *    getCommandByName(const std::string &name);
	void			addCommand(NetCommand *cmd);
};
typedef ACE_Singleton<NetCommandManager,ACE_Thread_Mutex> NetCommandManagerSingleton; // AdminServer Interface
