/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#ifndef MPRPC_GameServer_client_b5cc02dd_HPP__
#define MPRPC_GameServer_client_b5cc02dd_HPP__

#include "GameServer.hpp"

namespace StickWorld {

namespace GameServer {


class client : public msgpack::rpc::client::base {
public:
	client(const msgpack::rpc::address& addr, msgpack::rpc::loop lo = msgpack::rpc::loop()) :
		msgpack::rpc::client::base(addr, lo) { }

	client(const std::string& host, uint16_t port, msgpack::rpc::loop lo = msgpack::rpc::loop()) :
		msgpack::rpc::client::base(host, port, lo) { }

	~client() { }

	int32_t login_apply(
			const GameServer::login& message) {
		return instance.call_apply("login", message).get<int32_t>();
	}

	int32_t login(
			const std::string& login, const std::string& passw) {
		GameServer::login _Message;
		_Message.login = login;
		_Message.passw = passw;
		return login_apply(_Message);
	}

	msgpack::rpc::future::type<int32_t> login_async_apply(
			const GameServer::login& message) {
		return instance.call_apply("login", message);
	}

	msgpack::rpc::future::type<int32_t> login_async(
			const std::string& login, const std::string& passw) {
		GameServer::login _Message;
		_Message.login = login;
		_Message.passw = passw;
		return login_async_apply(_Message);
	}
};


}  // namespace GameServer


}  // namespace StickWorld
#endif
