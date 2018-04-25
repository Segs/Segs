#!/usr/bin/env ruby
# SEGS - Super Entity Game Server
# http://www.segs.io/
# Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
# This software is licensed! (See License.txt for details)

require 'EventProcessor'
class ChatConnection < EventProcessor
	def initialize()
		super
	end
	def on_network_bytes(source,bytes)
		
	end
end
class ChatHandler
	def on_subscribe(sub_evt)
	end
	def on_publish(pub_evt)		
	end
	def on_unsubscribe(unsub_evt)
	end
end
class ChatServer < EventProcessor
end
