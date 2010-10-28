
#include "GameServer_server.hpp"
#include <memory>
#include <mp/unordered_map.h>

namespace StickWorld {

namespace GameServer {


static void dispatch_login(server* svr, msgpack::rpc::request* preq)
{
	GameServer::login message;
	preq->params().convert(&message);
	svr->login(*preq, message);
}

typedef mp::unordered_map<std::string, void (*)(server*, msgpack::rpc::request*)> table_type;
#define TABLE server::s_dispatch_table.pimpl
server::dispatch_table server::s_dispatch_table;

server::dispatch_table::dispatch_table()
{
	std::auto_ptr<table_type> table(new table_type());
	table->insert(std::make_pair("login", &dispatch_login));
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


}  // namespace GameServer

}  // namespace StickWorld
