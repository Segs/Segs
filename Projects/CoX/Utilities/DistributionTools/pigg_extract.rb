#!/usr/bin/env ruby
# SEGS - Super Entity Game Server
# http://www.segs.io/
# Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
# This software is licensed! (See License.txt for details)

require 'zlib'
require 'pp'
require 'digest/md5'
require 'stringio'
class IO
	def readInt16()
		res1 = read(2);
		res1.unpack("S")[0]
	end

	def readInt32()
		res1 = read(4);
		res1.unpack("L")[0]
	end
end

class StringIO
	def readInt16()
		res1 = read(2);
		res1.unpack("S")[0]
	end 

	def readInt32()
		res1 = read(4);
		res1.unpack("L")[0]
	end 
end

class PiggFileItem
	attr_accessor :File,:name_id,:Index,:FileSize,:FileOffset,:FileSizeCompressed
	attr_accessor :Ver,:FileName,:FilePath;
	attr_reader :PackedFile
	attr_reader :src_file

	def initialize(filename)
		@src_file = filename
	end

	def ReadHeader(stream)
		@flag					= stream.readInt32()
		@name_id				= stream.readInt32()
		@FileSize				= stream.readInt32()
		@time_t					= Time.at(stream.readInt32())
		@FileOffset 			= stream.readInt32()
		@Ver					= stream.readInt32()
		@x 	= stream.readInt32() # CheckitCheckit!!!
		@md5_sum = stream.read(16)
		@FileSizeCompressed 	= stream.readInt32() # CheckitCheckit!!!
	end

	def GetData()
		zipper = Zlib::Inflate.new
		stream = File.open(@src_file,"rb");
		stream.binmode
		stream.seek(@FileOffset)
		size = (@FileSizeCompressed>0) ? @FileSizeCompressed : @FileSize
		buf = stream.read(size)
		stream.close
		if(@FileSizeCompressed==0)
			res= buf
		else
			res= Zlib::Inflate.inflate(buf)
		end
		res
	end

	def to_s
		@FileName
	end
end

class PiggDir
	attr_accessor :name

	def initialize(name)
		@m_children = Hash.new
		@files = Array.new
		@name = name
	end

	def add_child(dir)
		@m_children[dir.name] = dir
	end

	def add_file(item)
		@files<<item
	end

	def getDir(path_array)
		name = path_array.shift
		return self if(name==nil)
		add_child(PiggDir.new(name)) if @m_children[name]==nil
		
		return @m_children[name].getDir(path_array)
	end

	def save_hierarchy
		@tried = false
		begin
		Dir.chdir(@name) {
			@m_children.each_value {|child| 
				child.save_hierarchy
			}
			@files.each {|f|
				File.open(f.FileName,"wb") {|file| print("extracting:"+f.FileName+"\n"); file<<f.GetData()}
			}
		}
		rescue SystemCallError => e
			if e.instance_of?(Errno::ENOENT) then
				if @tried == false then
					Dir.mkdir(@name)
					@tried = true
					retry
				end
			end
		rescue Exception => e
			p e
		end
	end
end

class Collection
	def initialize
		@root_dir = PiggDir.new(".")
	end

	def AddFile(pigg_file)
		@current_dir = @root_dir
		dir = pigg_file.FilePath.index('/')
		path = pigg_file.FilePath
		dirs = path.split('/')
		pigg_file.FileName = dirs.pop
		cd dirs
		@current_dir.add_file(pigg_file)
	end

	def cd(dir_array)
		@current_dir = @current_dir.getDir(dir_array)
	end

	def save(top_path)		
		@root_dir.name = top_path;
		@root_dir.save_hierarchy()
	end
end

class PigFile
	attr_reader :m_fileName,:m_PiggFileItems

	def initialize
		@pool1={}
		@pool2={}
		@m_collection = Collection.new
	end

	def Load(name)
		@m_fileName = name
		@m_PiggFileItems=0
		stream = File.open(name);
		stream.binmode
		return false if !LoadHeader(stream)
		return false if !LoadFileInfoItems(stream)
		@pool1=LoadPool(stream)
		@pool2=LoadPool(stream)
		return false if !AssignFileNames()
	end

	def LoadFileInfoItems(stream)
      	@m_PiggFileItems = []
		0.upto(@num_entries-1) do
			item1 = PiggFileItem.new(File.expand_path(@m_fileName).gsub('/','\\'))
			item1.ReadHeader(stream);
            @m_PiggFileItems<<item1;
		end
      return true;
	end

	def split_into_tables(pool)
		res = []
		s = StringIO.new(pool['entries'])
		while(!s.eof?)
			slen = s.readInt32
			res << s.read(slen).rstrip
		end
		res
	end

	def LoadPool(stream)
		pool={}
		pool['flag'] = stream.readInt32
		pool['table_size'] = stream.readInt32
		pool['num_entries'] = stream.readInt32
		pool['entries'] = stream.read(pool['num_entries'])
		split_into_tables(pool)
	end

	def AssignFileNames()
		@m_PiggFileItems.each { |item| 
			item.FilePath= @pool1[item.name_id]
			@m_collection.AddFile(item)
		}
		return true
	end

	def LoadHeader(stream)
		pigg_magic = stream.readInt32()
		@a = stream.readInt16()
		@pigg_version = stream.readInt16()
		@header_size = stream.readInt16()
		@used_header_bytes = stream.readInt16()
		@num_entries = stream.readInt32
		
		return false if pigg_magic!=0x123
		if ( @header_size != 16 )
			p "header larger then 16 bytes"
			stream.seek(@header_size - 16, SEEK_CUR) 
		end
		return true
	end

	def save_all(top_dir)
		@m_collection.save(top_dir)
	end
end;

def extract_pigg(pigg,target_dir)
	t = PigFile.new
	t.Load(pigg)
	t.save_all(target_dir)
end

def extract_all
	Dir.foreach('./piggs') { |f|
		next if File.basename(f)!='bin.pigg'
		extract_pigg(File.join('piggs',f),'./piggs_unbound')
	}
end
extract_all()
