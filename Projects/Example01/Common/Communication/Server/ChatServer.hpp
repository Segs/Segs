#ifndef MPRPC_ChatServer_e916179d_HPP__
#define MPRPC_ChatServer_e916179d_HPP__

#include "types.hpp"

namespace StickWorld {

namespace ChatServer {


struct subscribe {

	subscribe()
		:
		my_id(0)
	{
	}

	int64_t my_id;

	template <typename Packer>
	void msgpack_pack(Packer& _Pk) const {
		_Pk.pack_array(1);
		_Pk.pack(my_id);
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
				_Array[0].convert(&my_id);
			}

	}
};
struct publish {

	publish()
	{
	}

	ChatMessage x;

	template <typename Packer>
	void msgpack_pack(Packer& _Pk) const {
		_Pk.pack_array(1);
		_Pk.pack(x);
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
				_Array[0].convert(&x);
			}

	}
};
struct unsubscribe {

	unsubscribe()
		:
		my_id(0)
	{
	}

	int64_t my_id;

	template <typename Packer>
	void msgpack_pack(Packer& _Pk) const {
		_Pk.pack_array(1);
		_Pk.pack(my_id);
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
				_Array[0].convert(&my_id);
			}

	}
};


}  // namespace ChatServer

}  // namespace StickWorld
#endif
