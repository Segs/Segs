/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#ifndef MPRPC_ChatClient_server_996f62ea_HPP__
#define MPRPC_ChatClient_server_996f62ea_HPP__

#include "ChatClient.hpp"

namespace StickWorld {

namespace ChatClient {


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
	void notify(msgpack::rpc::request::type<void>, ChatClient::notify&);
};


}  // namespace ChatClient


/*
void ChatClient::server::notify(msgpack::rpc::request::type<void> req, ChatClient::notify& params)
*/


}  // namespace StickWorld
#endif
