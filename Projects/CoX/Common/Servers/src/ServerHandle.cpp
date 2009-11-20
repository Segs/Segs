#include "ServerHandle.h"

// Constructors/Destructors
//  
/**
	Constructs 'local' server handle, attempt to pass it to remote server will result in X
*/
template<class SERVER_CLASS> 
ServerHandle<SERVER_CLASS>::ServerHandle(SERVER_CLASS *impl ) : m_server(impl),m_address(),m_id(-1) {};

/**
	Constructs 'remote' server handle, this can be passed to local server ( InterfaceManager will identify proper local server and fill m_server)
	otherwise InterfaceManager will connect to given address, check if server with given id exists, and create interface object of *RemoteInterface class.
*/
template<class SERVER_CLASS> 
ServerHandle<SERVER_CLASS>::ServerHandle(const ACE_INET_Addr &addr,int id) : m_server(0),m_address(addr),m_id(id) {};

/**
 * Set the value of m_address
 * Address of remote host.
 * @param new_var the new value of m_address
 */
template<class SERVER_CLASS> 
void ServerHandle<SERVER_CLASS>::setAddress ( ACE_INET_Addr new_var )
{
    m_address = new_var;
}

/**
 * Get the value of m_address
 * Address of remote host.
 * @return the value of m_address
 */
template<class SERVER_CLASS> 
ACE_INET_Addr ServerHandle<SERVER_CLASS>::getAddress ( )
{
    return m_address;
}

/**
 * Set the value of m_id
 * Remote host uses this to identify it's servers.
 * @param new_var the new value of m_id
 */
template<class SERVER_CLASS> 
void ServerHandle<SERVER_CLASS>::setId ( int new_var ) {
    m_id = new_var;
}

/**
 * Get the value of m_id
 * Remote host uses this to identify it's servers.
 * @return the value of m_id
 */
template<class SERVER_CLASS> 
int ServerHandle<SERVER_CLASS>::getId ( ) {
    return m_id;
}

/**
 * Set the value of m_server
 * Set only if the handle is for same process server (i.e. thread)
 * @param new_var the new value of m_server
 */
template<class SERVER_CLASS> 
void ServerHandle<SERVER_CLASS>::setServer ( SERVER_CLASS * new_var ) {
    m_server = new_var;
}

/**
 * Get the value of m_server
 * Set only if the handle is for same process server (i.e. thread)
 * @return the value of m_server
 */
template<class SERVER_CLASS> 
SERVER_CLASS *ServerHandle<SERVER_CLASS>::getServer()
{
    return m_server;
}
class IAuthServer;
class IAdminServer;
class IGameServer;
class IMapServer;

template class ServerHandle<IAuthServer>;
template class ServerHandle<IAdminServer>;
template class ServerHandle<IGameServer>;
template class ServerHandle<IMapServer>;
