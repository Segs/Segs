
class AuthServer
	# low level events, posted by network layer
	def on_packet(ev)
		cl=@clients.find_link_ip(ev.src_addr)
		if(cl==nil)
			return
		end
		post_event bytes_to_event(cl,ev.bytes)
	end
	def bytes_to_event(cl,bytes)
		res = nil
		
		res.link = cl
	end
	def on_connect(ev)
		cl=@clients.find_link_ip(ev.link.src_addr)
		raise "Multiple connection attempts from the same addr/port" if(cl!=nil)
		client_link = @clients.create_client_link(ev.link)
		client_link.state = :connected
		client_link.post_event AuthorizationProtocolVersion.new(self,30206)
	end
	def on_disconnect(ev)
		if(ev.link.state!=:waiting_for_disconnect)
			# premature disconnect
		end
		ev.link.state = :disconnected
	end
	# self posted events	
	def on_auth_request(ev)
		client_link = ev.link
		if(client_link.state != :connected)
			client_link.post_event AuthError.new(self,2,AuthServer::AUTH_UNKN_ERROR)
			return
		end
		auth_status = auth(ev.username,ev.password,ev.seed)
		if(auth_status == :ok)
			client_link.state = :logged_in
			client_link.post_event AuthSuccess.new(self)
		else
			client_link.post_event AuthError.new(self,2,auth_status)
		end
	end
	
	def on_server_list_request(ev)
		client_link = ev.link
		if(client_link.state != :logged_in)
			client_link.post_event AuthError.new(self,2,AuthServer::AUTH_UNKN_ERROR)
			return
		end
		server_list_event = ServerList.new(self)
		@known_game_servers.each {|gs|
			server_list_event.add_server(gs.id,gs.address,gs.current_players,gs.max_players,gs.online)
		}
		client_link.state = :deciding_on_server # now client will select a server
		client_link.post_event server_list_event		
	end
	def on_server_selected(ev)
		# client selected a server
		# we ask the selected server to give us a cookie
		if(ev.link.state != :deciding_on_server)
			ev.link.post_event AuthError.new(self,2,AuthServer::AUTH_UNKN_ERROR)
			return
		end
		game_server = @known_game_servers[ev.serverId]
		if(game_server == nil)
			ev.link.post_event AuthError.new(self,2,AuthServer::AUTH_UNKN_ERROR)
			return
		end
		ev.link.state = :waiting_for_cookies
		game_server.post_event ExpectClient.new(self,ev.link.addr,ev.link.client_id,ev.link.access_level)
		# as can be seen, we do not send anything to the client from here
	end
	# event posted by Game server
	def on_expecting_client(ev) 
		# now game server is expecting the client
		# we can post the relevant cookies to it
		client_link = @clients.find_by_id(ev.client_id)
		if(client_link==nil)
			# client might have disappeared before we receive response from GameServer
			return
		end
		if(client_link.state!=:waiting_for_cookies)
			client_link.post_event AuthError.new(self,2,AuthServer::AUTH_UNKN_ERROR)
			return
		end
		client_link.state = :waiting_for_disconnect
		client_link.post_event ServerResponse.new(self,ev.client,ev.db_cookie,ev.server_cookie)
	end
end
