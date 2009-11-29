#!/usr/bin/env ruby
require 'digest/md5'
digester=Digest::MD5.new
fp=File.open("digests","w")
Dir.foreach('.') {|f|
	printf(f+"\n")
	next if File.directory?(f)
	fp<<"#{f} - #{Digest::MD5.file(f)}\n"
}
Dir.foreach('./piggs') {|f|
	printf(f+"\n")
	next if File.directory?(f)
	fp<<"./piggs/#{f} - #{Digest::MD5.file('./piggs/'+f)}\n"
}
exit()
Dir.foreach('./piggs') {|f|
	next if File.directory?(f)
	noext=f[0..(f.index(File.extname(f))-1)]
	basename=File.basename(f)
	system "xdelta3.0u.x86-32.exe -v -S djw -9 -e -s piggs/#{basename} piggs_new/#{basename} #{noext}.xpatch"
}