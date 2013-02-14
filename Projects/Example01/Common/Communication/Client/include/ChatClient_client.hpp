#ifndef MPRPC_ChatClient_client_6fd354dd_HPP__
#define MPRPC_ChatClient_client_6fd354dd_HPP__

#include "ChatClient.hpp"

namespace StickWorld {

namespace ChatClient {


class client : public msgpack::rpc::client::base {
public:
	client(const msgpack::rpc::address& addr, msgpack::rpc::loop lo = msgpack::rpc::loop()) :
		msgpack::rpc::client::base(addr, lo) { }

	client(const std::string& host, uint16_t port, msgpack::rpc::loop lo = msgpack::rpc::loop()) :
		msgpack::rpc::client::base(host, port, lo) { }

	~client() { }

	void notify_apply(
			const ChatClient::notify& message) {
		instance.call_apply("notify", message).get<void>();
	}

	void notify(
			const ChatMessage& x) {
		ChatClient::notify _Message;
		_Message.x = x;
		return notify_apply(_Message);
	}

	msgpack::rpc::future::type<void> notify_async_apply(
			const ChatClient::notify& message) {
		return instance.call_apply("notify", message);
	}

	msgpack::rpc::future::type<void> notify_async(
			const ChatMessage& x) {
		ChatClient::notify _Message;
		_Message.x = x;
		return notify_async_apply(_Message);
	}
};


}  // namespace ChatClient


}  // namespace StickWorld
#endif
