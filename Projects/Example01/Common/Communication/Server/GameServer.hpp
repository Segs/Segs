#ifndef MPRPC_GameServer_48179120_HPP__
#define MPRPC_GameServer_48179120_HPP__

#include "types.hpp"

namespace StickWorld {

namespace GameServer {


struct login {

	login()
	{
	}

	std::string login;
	std::string passw;

	template <typename Packer>
	void msgpack_pack(Packer& _Pk) const {
		_Pk.pack_array(2);
		_Pk.pack(login);
		_Pk.pack(passw);
	}

	void msgpack_unpack(msgpack::object _Obj) {
		if(_Obj.type != msgpack::type::ARRAY) {
			throw msgpack::type_error();
		}
		const size_t _Length = _Obj.via.array.size;
		msgpack::object* const _Array = _Obj.via.array.ptr;

		if(_Length < 0) {
			throw msgpack::type_error();
		}


			if(_Length <= 0) { return; }
			if(!_Array[0].is_nil()) {
				_Array[0].convert(&login);
			}


			if(_Length <= 1) { return; }
			if(!_Array[1].is_nil()) {
				_Array[1].convert(&passw);
			}

	}
};


}  // namespace GameServer

}  // namespace StickWorld
#endif
