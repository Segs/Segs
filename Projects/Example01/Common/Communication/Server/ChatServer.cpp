/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup ExampleChatServer Projects/Example01/Common/Communication/Server
 * @{
 */

#include "ChatServer_server.hpp"
#include <memory>
#include <mp/unordered_map.h>

namespace StickWorld {

namespace ChatServer {


static void dispatch_subscribe(server* svr, msgpack::rpc::request* preq)
{
	ChatServer::subscribe message;
	preq->params().convert(&message);
	svr->subscribe(*preq, message);
}
static void dispatch_publish(server* svr, msgpack::rpc::request* preq)
{
	ChatServer::publish message;
	preq->params().convert(&message);
	svr->publish(*preq, message);
}
static void dispatch_unsubscribe(server* svr, msgpack::rpc::request* preq)
{
	ChatServer::unsubscribe message;
	preq->params().convert(&message);
	svr->unsubscribe(*preq, message);
}

typedef mp::unordered_map<std::string, void (*)(server*, msgpack::rpc::request*)> table_type;
#define TABLE server::s_dispatch_table.pimpl
server::dispatch_table server::s_dispatch_table;

server::dispatch_table::dispatch_table()
{
	std::auto_ptr<table_type> table(new table_type());
	table->insert(std::make_pair("subscribe", &dispatch_subscribe));
	table->insert(std::make_pair("publish", &dispatch_publish));
	table->insert(std::make_pair("unsubscribe", &dispatch_unsubscribe));
	TABLE = (void*)table.release();
}

server::dispatch_table::~dispatch_table()
{
	delete (table_type*)TABLE;
}

void server::dispatch(msgpack::rpc::request req)
try {
	std::string method;
	req.method().convert(&method);

	const table_type* table((table_type*)TABLE);

	table_type::const_iterator m = table->find(method);
	if(m == table->end()) {
		req.error(msgpack::rpc::NO_METHOD_ERROR);
		return;
	}

	(*m->second)(this, &req);

} catch (msgpack::type_error& e) {
	req.error(msgpack::rpc::ARGUMENT_ERROR);
	return;

} catch (std::exception& e) {
	req.error(std::string(e.what()));
	return;
}


}  // namespace ChatServer

}  // namespace StickWorld

//! @}
