#!/usr/bin/env ruby
require 'pp'
require 'CMakeCreator.rb'
require 'SegsUtil.rb'
def show_usage()
    print("usage:\nCreateProject name\n")
    print("\tThis utility will create skeleton for the named project \n")
end
class Servers_Creator
    def initialize(parent_cmake,prefix,*servers)
        @prefix=prefix
        @servers_arr=servers
        Dir.mkdir_optional(File.join(parent_cmake.current_path,"Servers")) if @servers_arr.size>0
        parent_cmake.add_subdir("Servers")
        @cmake = CMakeFile.new("Servers",parent_cmake)
    end
    def create()
        @servers_arr.each {|serv| create_server(serv) }
    end
    def create_server(serv)
        @cmake.add_subdir(serv,@prefix,true)
        serv_path = File.join(@cmake.current_path,serv)
        Dir.mkdir_optional(serv_path)
        Dir.mkdir_optional(File.join(serv_path,"src"))
        Dir.mkdir_optional(File.join(serv_path,"include"))
        Dir.mkdir_optional(File.join(serv_path,"docs"))
        CMakeFile.new(serv,@cmake).create_component(@prefix+"_"+serv)
    end
end
class Clients_Creator
    def initialize(parent_cmake,prefix,*clients)
        @prefix=prefix
        @clients=clients
        parent_cmake.add_subdir("Clients")
        Dir.mkdir_optional(File.join(parent_cmake.current_path,"Clients")) if @clients.size>0
        @cmake = CMakeFile.new("Clients",parent_cmake)
    end
    def create()
        sub_dir = @cmake.current_path
        @clients.each {|client| create_client(client) }
    end
    def create_client(name)
        @cmake.add_subdir(name,@prefix,true)
        path = File.join(@cmake.current_path,name)
        Dir.mkdir_optional(path)
        Dir.mkdir_optional(File.join(path,"src"))
        Dir.mkdir_optional(File.join(path,"include"))
        Dir.mkdir_optional(File.join(path,"docs"))
        CMakeFile.new(name,@cmake).create_component(@prefix+"_"+name)
    end
end
class Common_Creator
    def initialize(parent_cmake,prefix,topname,*subs)
        @prefix=prefix
        @subs=subs
        parent_cmake.add_subdir(topname)
        Dir.mkdir_optional(File.join(parent_cmake.current_path,topname))
        @cmake = CMakeFile.new(topname,parent_cmake)
    end
    def create()
        sub_dir = @cmake.current_path
        @subs.each {|sub| create_sub(sub) }
    end
    def create_sub(name)
        @cmake.add_subdir(name,@prefix,true)
        path = File.join(@cmake.current_path,name)
        Dir.mkdir_optional(path)
        Dir.mkdir_optional(File.join(path,"src"))
        Dir.mkdir_optional(File.join(path,"include"))
        Dir.mkdir_optional(File.join(path,"docs"))
        CMakeFile.new(name,@cmake).create_component(@prefix+"_"+name)
    end
end
class Data_Creator
    def initialize(cmake,*servers)
        @servers_arr=servers
        @cmake = cmake
    end
end
class Docs_Creator
    def initialize(cmake,*servers)
        @servers_arr=servers
        @cmake = cmake
    end
end

if(ARGV.size()!=1)
    show_usage()
else
    Dir.chdir("../Projects")
    root_cmake = CMakeFile.new(".") # Top dir in Projects
    project_dir = File.join(root_cmake.current_path,ARGV[0])
    Dir.mkdir_optional(project_dir)

    project_root = CMakeFile.new(project_dir,root_cmake)
    root_cmake.add_subdir(ARGV[0],"",true)
    sc = Servers_Creator.new(project_root,ARGV[0],"TestServer","LoginServer")
    sc.create()
    cc = Clients_Creator.new(project_root,ARGV[0],"CliClient","GuiClient")
    cc.create()
    Common_Creator.new(project_root,ARGV[0],"Common").create()
    Common_Creator.new(project_root,ARGV[0],"Utilities").create()
    Common_Creator.new(project_root,ARGV[0],"Data").create()
    Common_Creator.new(project_root,ARGV[0],"Docs").create()
end


