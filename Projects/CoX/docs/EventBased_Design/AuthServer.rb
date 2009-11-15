
class AuthServer
	# low level events, posted by network layer
	def on_packet(ev)
		cl=@clients.find_link_ip(ev.src_addr)
		if(cl==nil)
			return
		end
		post_event bytes_to_event(cl,ev.bytes)
	end
	def on_connect(ev)
		cl=@clients.find_link_ip(ev.link.src_addr)
		raise "Multiple connection attempts from the same addr/port" if(cl!=nil)
		client_link = @clients.create_client_link(ev.link)
		client_link.state = :connected
		client_link.post_event AuthorizationProtocolVersion.new(self,30206)
	end
	# self posted events	
	def on_auth_request(ev)
		client_link = @clients.find_link_ip(ev.src_addr)
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
		client_link = @clients.find_link_ip(ev.src_addr)
		if(client_link.state != :logged_in)
			client_link.post_event AuthError.new(self,2,AuthServer::AUTH_UNKN_ERROR)
		end		
	end
	
	def on_expecting_client(ev) # event posted by Game server
		# now game server is expecting the client
		# we can post the relevant packet to it
		ev.client.post_event ServerResponse.new(self,ev.client,ev.db_cookie,ev.server_cookie)
	end
end
