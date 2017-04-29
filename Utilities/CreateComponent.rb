#!/usr/bin/env ruby
require 'pp'
require 'CMakeCreator.rb'
def show_usage()
    print("usage:\nCreateComponent component_type component_name\n")
    print("\tWhere component_type is one of SERVER|CLIENT|COMMON\n")
    
end
def create_component(type,name)
    sub_dir=""
    case(type.upcase)
        when "SERVER"
            sub_dir="Server"
        when "COMMON"
            sub_dir="Common"
        when "CLIENT"
            sub_dir="Client"
    end
    root_cmake = CMakeFile.new(sub_dir)
    component_path = File.join(sub_dir,name)
    Dir.mkdir_optional(component_path)
    Dir.mkdir_optional(File.join(component_path,"src"))
    Dir.mkdir_optional(File.join(component_path,"include"))
    Dir.mkdir_optional(File.join(component_path,"docs"))

    root_cmake.add_subdir(name)

    cm = CMakeFile.new(component_path,root_cmake)
    cm.create_component(name)
end
if(ARGV.size()!=2)
    show_usage()
else
    Dir.chdir("../Components")
    create_component(ARGV[0],ARGV[1])
    Dir.chdir("../Utilities")
end