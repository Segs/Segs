class GameServer
	def on_expect_new_client(ev) # posted by AuthServer
		db_cookie 					 = create_dbcookie(ev.client)
		server_cookie 				 = create_serv_cookie(ev.client)
		@expected_clients[db_cookie] = ev.client
		ev.src.post_event ExpectingClient.new(self,db_cookie,server_cookie)
	end
	def on_server_update(ev)
		client = expected_clients(ev.src_addr,ev.cookie)
		if(ev.build_date!=SUPPORTED_VERSION)
			client.post_event WrongVersionEvent.new(self,ev.build_date)
			return
		end
		if(client==nil)
			post_event UnauthorizedConnectionEvent.new(ev.src)
		end
		client.target = self  # all events targeted to the client will be posted
							  # to us
		if false==client.serialize_from_db()
			post_event DbErrEvent.new(ev.src)
		else
			@m_clients[ev.src_addr] = client
			client.post_event SlotListEvent.new(self,client)
		end
	end
	def on_expecting_client(ev) # event posted by MapServer
		
	end
	def on_map_query(ev)
		if(@map_servers.size>0)
			@map_servers[0].post_event ExpectClient.new(self,ev.client)
		end
	end
	def on_data(ev)
		
		client = @m_clients
	end
end
