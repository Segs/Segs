#ifndef MPRPC_GameServer_server_6abc2380_HPP__
#define MPRPC_GameServer_server_6abc2380_HPP__

#include "GameServer.hpp"

namespace StickWorld {

namespace GameServer {


class server : public msgpack::rpc::server::base {
public:
	server(msgpack::rpc::loop lo = msgpack::rpc::loop()) :
		msgpack::rpc::server::base(lo) { }

	~server() { }

	void dispatch(msgpack::rpc::request req);

private:
	class dispatch_table {
	public:
		dispatch_table();
		~dispatch_table();
		void* pimpl;
	};

	static dispatch_table s_dispatch_table;
	friend class dispatch_table;

public:
	void login(msgpack::rpc::request::type<int32_t>, GameServer::login&);
};


}  // namespace GameServer


/*
void GameServer::server::login(msgpack::rpc::request::type<int32_t> req, GameServer::login& params)
*/


}  // namespace StickWorld
#endif
