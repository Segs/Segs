#!/usr/bin/env ruby
require 'EventProcessor'
class MapConnection < EventProcessor
	def on_network_bytes(source,bytes)
		
	end
end
class MapHandler
	def on_movement(move_event)
	end
	def on_map_enter(move_event)
	end
	def on_map_leave(move_event)
	end
end
class MapServer < EventProcessor
	def initialize
		@connections =[]
		@map_instance=nil
	end
	def on_accept(c)
		@connections << MapConnection.new(MapHandler.new(@map_instance),c)
	end
end
