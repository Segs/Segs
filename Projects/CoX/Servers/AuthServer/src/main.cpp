/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: main.cpp 316 2007-01-25 15:17:16Z nemerle $
 */

#include <iostream>
#include <stdlib.h>
#define ACE_NTRACE 0
#include <ace/ACE.h>
#include <ace/Get_Opt.h>
#include <ace/OS.h>
#include <ace/Singleton.h>
#include <ace/Null_Mutex.h>
#include <ace/Log_Msg.h>
#include <ace/Log_Msg_Callback.h>
#include <ace/Log_Record.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Acceptor.h>

#include <ace/Reactor.h>
#include <ace/TP_Reactor.h>
#include <ace/Log_Msg.h>
#include <ace/INET_Addr.h>
#include <ace/Reactor.h>
#include <ace/OS_main.h> //Included to enable file logging
#include <ace/streams.h> //Included to enable file logging 

//#include "Auth.h"
#include "AuthClientServicer.h"
#include "ServerManager.h"
#include "server_support.h"
#include "version.h"
//////////////////////////////////////////////////////////////////////////

#include "AdminServer.h"
#include "AuthServer.h"
#include "MapServer.h"
#include "GameServer.h"
//////////////////////////////////////////////////////////////////////////

class LogCallback : public ACE_Log_Msg_Callback
{
	std::ofstream m_tgt_file_stream;
	ACE_Thread_Mutex m_log_lock;
public:
	LogCallback() : ACE_Log_Msg_Callback()
	{
		
	}
	virtual ~LogCallback()
	{
		m_tgt_file_stream.close();
	}
	void init_file_log() // no need for error checking at this point
	{
		// Create a persistent store.
		// Lets setup logging to a file in addtion to logging to the console
#if defined (ACE_LACKS_IOSTREAM_TOTALLY)
		return;
#endif
		ACE_Guard<ACE_Thread_Mutex> locker(m_log_lock);
		if(m_tgt_file_stream.is_open())
			return;
		const char *filename = "output.log";
		m_tgt_file_stream.open(filename, ios::out | ios::trunc);
		if(m_tgt_file_stream.bad())
			ACE_DEBUG((LM_WARNING,ACE_TEXT("Failed to open file for logging.\n")));
	}
	virtual void log (ACE_Log_Record &log_record)
	{
		ACE_Guard<ACE_Thread_Mutex> locker(m_log_lock);
		log_record.print (ACE_TEXT (""), 0, std::cerr);
		if (!m_tgt_file_stream.bad ())
		{
			log_record.print (ACE_TEXT (""), ACE_Log_Msg::VERBOSE, m_tgt_file_stream);
		}
	}
};
LogCallback g_logging_object;

static ACE_THR_FUNC_RETURN event_loop (void *arg) 
{ 
	ACE_Reactor *reactor = ACE_static_cast (ACE_Reactor *, arg); 
	ACE_LOG_MSG->clr_flags(ACE_Log_Msg::STDERR);
	ACE_LOG_MSG->set_flags(ACE_Log_Msg::MSG_CALLBACK);
	ACE_LOG_MSG->priority_mask (LM_DEBUG |LM_ERROR | LM_WARNING| LM_NOTICE | LM_INFO , ACE_Log_Msg::PROCESS);
	ACE_LOG_MSG->msg_callback(&g_logging_object);

	reactor->owner (ACE_OS::thr_self ()); 
	reactor->run_reactor_event_loop (); 
	return 0; 
}
static bool CreateServers()
{
	GameServer *game_instance		= new GameServer;
	MapServer * map_instance		= new MapServer;
	ServerManager::instance()->SetAdminServer(AdminServer::instance());
	ServerManager::instance()->SetAuthServer(new AuthServer);
	ServerManager::instance()->AddGameServer(game_instance);
	ServerManager::instance()->AddMapServer(map_instance);
	return true;
};

ACE_INT32 ACE_TMAIN (int argc, ACE_TCHAR *argv[])
{
//	int opt;
	const size_t N_THREADS = 4;
	ACE_TP_Reactor threaded_reactor; 
	ACE_Reactor new_reactor(&threaded_reactor); //create concrete reactor
	auto_ptr<ACE_Reactor> old_instance(ACE_Reactor::instance(&new_reactor)); // this will delete old instance when app finishes
	
	ServerStopper st; // it'll register itself with current reactor, and shut it down on sigint
	g_logging_object.init_file_log();
	ACE_LOG_MSG->clr_flags(ACE_Log_Msg::STDERR);
	ACE_LOG_MSG->set_flags(ACE_Log_Msg::MSG_CALLBACK);
	ACE_LOG_MSG->priority_mask (LM_DEBUG |LM_ERROR | LM_WARNING| LM_NOTICE | LM_INFO , ACE_Log_Msg::PROCESS);
	ACE_LOG_MSG->msg_callback(&g_logging_object);

	// Print out startup copyright messages
	VersionInfo version;
	version.getAuthVersion();
	version.getCopyright();
	ACE_DEBUG((LM_ERROR,ACE_TEXT("main\n")));
/*
	AuthServerTP server_pool;
	server_pool.activate(THR_NEW_LWP|THR_JOINABLE,4);
*/
	ACE_Thread_Manager::instance()->spawn_n(N_THREADS, event_loop, ACE_Reactor::instance()); 
	bool no_err=true;
	no_err=CreateServers();
	no_err=ServerManager::instance()->LoadConfiguration(".");
	if(no_err)
		no_err=ServerManager::instance()->StartLocalServers();
	if(no_err)
		no_err=ServerManager::instance()->CreateServerConnections();
	if(!no_err)
	{
		ACE_Reactor::instance()->end_event_loop();
		ACE_Thread_Manager::instance()->wait();
		return -1;
	}
	ACE_Thread_Manager::instance()->wait();
	return (0);
}

