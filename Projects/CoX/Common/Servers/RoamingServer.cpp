#include "RoamingServer.h"
#include "ServerHandle.h"
#include "InterfaceManager.h"
#include "ConfigExtension.h"
/**
 * @return int
 * @param  configpath This is a platform specific path to a config file containing
 * general RoamingServer vars.
 */
bool RoamingServer::ReadConfig(const std::string &inipath)
{
	StringsBasedCfg config;
	ACE_Ini_ImpExp	config_importer(config);
	ACE_Configuration_Section_Key root;
	if (config.open () == -1)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("%p\n"), ACE_TEXT ("config")),false);
	}
	if (config_importer.import_config (inipath.c_str()) == -1)
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) RoamingServer: Unable to open config file : %s\n"), inipath.c_str()),false);
	if(-1==config.open_section(config.root_section(),"RoamingServer",1,root))
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) RoamingServer: Config file %s is missing [RoamingServer] section\n"), inipath.c_str()),false);
	config.get_addr(root,ACE_TEXT("location_addr"),m_authaddr,ACE_INET_Addr(2106,"127.0.0.1"));
	config.get_string_value(root,ACE_TEXT("auth_pass"),m_passw,"");
	return true;
}

/**
* This method will return a valid AuthServerInterface, or NULL if this server
* fails to connect to the AuthServer.
* It'll build an AuthServer handle, and pass it to InterfaceManager, to resolve it to concrete Interface.
* 
* @return AuthServerInterface constructed during call
*/
AuthServerInterface *RoamingServer::getAuthServer ( ) 
{
	ServerHandle<IAuthServer> h_auth(m_authaddr,-1);
	return InterfaceManager::instance()->get(h_auth);

}
