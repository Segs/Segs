#!/usr/bin/env ruby
require 'EventProcessor'
class AuthConnection < EventProcessor
	def initialize()
		super
	end
	def on_auth_success(success_evt)
		success_evt
	end
end
class AuthHandler < EventProcessor
	def on_login(login_event)
		login_event.source.post_event(AuthSuccess.new('token'))
	end
end
class AuthServer 
end
