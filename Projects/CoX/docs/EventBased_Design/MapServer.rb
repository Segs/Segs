class MapServer
	def on_expect_client(ev)
		ev.src.post_event expect_client(ev.client)
	end
end
