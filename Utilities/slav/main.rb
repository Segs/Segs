#!/usr/bin/env ruby
=begin
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2011 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
=end
$LOAD_PATH.unshift File.dirname($0)
require 'Qt'
require 'updater'

app = Qt::Application.new(ARGV)
dialog = UpdaterForm.new
dialog.exec

