/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#ifndef MPRPC_TYPES_a732d5de_HPP__
#define MPRPC_TYPES_a732d5de_HPP__

#include <msgpack/rpc/client.h>
#include <msgpack/rpc/server.h>
#include <stdexcept>

namespace StickWorld {


struct ChatMessage {

	ChatMessage()
		:
		source_id(0),
		timestamp(0)
	{
	}

	int64_t source_id;
	int64_t timestamp;
	std::string message;

	template <typename Packer>
	void msgpack_pack(Packer& _Pk) const {
		_Pk.pack_array(3);
		_Pk.pack(source_id);
		_Pk.pack(timestamp);
		_Pk.pack(message);
	}

	void msgpack_unpack(msgpack::object _Obj) {
		if(_Obj.type != msgpack::type::ARRAY) {
			throw msgpack::type_error();
		}
		const size_t _Length = _Obj.via.array.size;
		msgpack::object* const _Array = _Obj.via.array.ptr;

		if(_Length < 3) {
			throw msgpack::type_error();
		}


			_Array[0].convert(&source_id);


			_Array[1].convert(&timestamp);


			_Array[2].convert(&message);

	}
};




}  // namespace StickWorld
#endif
