# SEGS - Super Entity Game Server
# http://www.segs.io/
# Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
# This software is licensed! (See License.txt for details)

class MapServer
	############################################################################
	# Events from network layer
	def on_connect(ev)
		# as MapServer is on crudp link
		# the on connect is actually result of a control packet beeing received.
		if(expecting_client_from(ev.addr))
			ev.src.post_event ConnectedEvent.new(self)
			@clients[ev.addr] = Client.new
			client.is_nice = false
		end
	end

	def on_bytes(ev)
		# only client events are passed to us as plain old bytes
		# unless we implement split server executables.
		client = @clients[ev.src_addr]
		# client can only be 0 if it's a connect packet
		post_event bytes_to_event(client,ev.bytes)
	end

	############################################################################
	# Events posted by GameServer
	def on_expect_client(ev)
		ev.src.post_event expect_client(ev.client)
	end
end

