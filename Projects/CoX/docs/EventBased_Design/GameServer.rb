class GameServer
	
	############################################################################
	# Events from network layer
	def on_connect(ev)
		# as GameServer is on crudp link
		# the on connect is actually result of a control packet beeing received.
		if(expecting_client_from(ev.addr))
			ev.src.post_event ConnectedEvent.new(self)
			@clients[ev.addr] = Client.new
			client.is_nice = false
		end
	end
	# only client events are passed to us as plain old bytes
	def on_bytes(ev)
		client = @clients[ev.src_addr]
		# client can only be 0 if it's a connect packet
		post_event bytes_to_event(client,ev.bytes)
	end

	############################################################################
	# Events from timer
	def on_tick(ev)
		# do something time related :)
		# maybe post connection keep-alive packets here
		# or maybe leave it to client objects ?
	end

	############################################################################
	# Events from AuthServer
	def on_expect_new_client(ev) 
		# ev.src is an AuthServer
		# ev.client_id is unique client number
		# ev.access_level is client's access level
		db_cookie 					 = create_dbcookie(ev.client_addr,ev.client_id)
		server_cookie 				 = create_serv_cookie(ev.client_addr,ev.client_id)
		@expected_clients[db_cookie] = Client.new(ev.client_addr,ev.client_id,ev.access_level)
		ev.src.post_event ExpectingClient.new(self,db_cookie,server_cookie)
	end

	############################################################################
	# Events from MapServer
	def on_expecting_client(ev) 
		# now map server is expecting the client
		# we can post the relevant cookies to it
		ev.client.post_event ServerResponse.new(self,ev.client.addr,ev.db_cookie,ev.server_cookie)
	end

	############################################################################
	# Events from the client
	def on_server_update(ev)
		if(cookies_are_tasty(ev.client,ev.cookie))
			ev.client.is_nice=true
			munch_on(ev.cookie)
		end
		if(not ev.client.is_nice)
			post_event UnauthorizedConnectionEvent.new(ev.src)
			return
		end
		if(ev.build_date!=SUPPORTED_VERSION)
			client.post_event WrongVersionEvent.new(self,ev.build_date)
			return
		end
		if(false==client.serialize_from_db())
			post_event DbErrEvent.new(ev.src)
		else
			client.post_event SlotListEvent.new(self,client)
		end
	end
	def on_update_character(ev)
		# client requests given update to given character slot
		character_data=ev.client.get_character(ev.char_idx)
		ev.client.post_event CharUpdateEvent.new(self,character_data)
	end
	def on_delete_character(ev)
		# client wants to delete given character
		character_data=ev.client.get_character(ev.char_idx)
		if(character_data and character_data.name == ev.name )
			ev.client.delete_character(ev.char_idx)
		end
		# after delete the clien will ask us for update, so no event is posted
	end
	def on_map_query(ev)
		cl = ev.client
		if(@map_servers.size>0)
			map_server_idx = ev.m_mapnumber
			ms = @map_servers[0] # just one map for now, always the same
			ms.post_event ExpectClient.new(self,cl.addr,ev.char_index,ev.char_name)
		end
		# client gets nothing back here
		# on_expecting_client will post proper response
	end
end
