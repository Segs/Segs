#!/usr/bin/env ruby
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
