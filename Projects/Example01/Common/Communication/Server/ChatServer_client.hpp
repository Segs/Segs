#ifndef MPRPC_ChatServer_client_d8ace49b_HPP__
#define MPRPC_ChatServer_client_d8ace49b_HPP__

#include "ChatServer.hpp"

namespace StickWorld {

namespace ChatServer {


class client : public msgpack::rpc::client::base {
public:
	client(const msgpack::rpc::address& addr, msgpack::rpc::loop lo = msgpack::rpc::loop()) :
		msgpack::rpc::client::base(addr, lo) { }

	client(const std::string& host, uint16_t port, msgpack::rpc::loop lo = msgpack::rpc::loop()) :
		msgpack::rpc::client::base(host, port, lo) { }

	~client() { }

	void subscribe_apply(
			const ChatServer::subscribe& message) {
		instance.call_apply("subscribe", message).get<void>();
	}

	void subscribe(
			const int64_t& my_id) {
		ChatServer::subscribe _Message;
		_Message.my_id = my_id;
		return subscribe_apply(_Message);
	}

	msgpack::rpc::future::type<void> subscribe_async_apply(
			const ChatServer::subscribe& message) {
		return instance.call_apply("subscribe", message);
	}

	msgpack::rpc::future::type<void> subscribe_async(
			const int64_t& my_id) {
		ChatServer::subscribe _Message;
		_Message.my_id = my_id;
		return subscribe_async_apply(_Message);
	}
	void publish_apply(
			const ChatServer::publish& message) {
		instance.call_apply("publish", message).get<void>();
	}

	void publish(
			const ChatMessage& x) {
		ChatServer::publish _Message;
		_Message.x = x;
		return publish_apply(_Message);
	}

	msgpack::rpc::future::type<void> publish_async_apply(
			const ChatServer::publish& message) {
		return instance.call_apply("publish", message);
	}

	msgpack::rpc::future::type<void> publish_async(
			const ChatMessage& x) {
		ChatServer::publish _Message;
		_Message.x = x;
		return publish_async_apply(_Message);
	}
	void unsubscribe_apply(
			const ChatServer::unsubscribe& message) {
		instance.call_apply("unsubscribe", message).get<void>();
	}

	void unsubscribe(
			const int64_t& my_id) {
		ChatServer::unsubscribe _Message;
		_Message.my_id = my_id;
		return unsubscribe_apply(_Message);
	}

	msgpack::rpc::future::type<void> unsubscribe_async_apply(
			const ChatServer::unsubscribe& message) {
		return instance.call_apply("unsubscribe", message);
	}

	msgpack::rpc::future::type<void> unsubscribe_async(
			const int64_t& my_id) {
		ChatServer::unsubscribe _Message;
		_Message.my_id = my_id;
		return unsubscribe_async_apply(_Message);
	}
};


}  // namespace ChatServer


}  // namespace StickWorld
#endif
