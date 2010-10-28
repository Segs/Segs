#ifndef MPRPC_ChatServer_server_7c6c320b_HPP__
#define MPRPC_ChatServer_server_7c6c320b_HPP__

#include "ChatServer.hpp"

namespace StickWorld {

namespace ChatServer {


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
	void subscribe(msgpack::rpc::request::type<void>, ChatServer::subscribe&);
	void publish(msgpack::rpc::request::type<void>, ChatServer::publish&);
	void unsubscribe(msgpack::rpc::request::type<void>, ChatServer::unsubscribe&);
};


}  // namespace ChatServer


/*
void ChatServer::server::subscribe(msgpack::rpc::request::type<void> req, ChatServer::subscribe& params)
void ChatServer::server::publish(msgpack::rpc::request::type<void> req, ChatServer::publish& params)
void ChatServer::server::unsubscribe(msgpack::rpc::request::type<void> req, ChatServer::unsubscribe& params)
*/


}  // namespace StickWorld
#endif
