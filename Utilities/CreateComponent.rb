#!/usr/bin/env ruby
require 'pp'
def show_usage()
    print("usage:\nCreateComponent component_type component_name\n")
    print("\tWhere component_type is one of SERVER|CLIENT|COMMON\n")
    
end
class Dir
    def Dir.mkdir_optional(name)
        if(File.exist?(name))
            print("Warning: directory #{name} already exists\n")
            return 0
        end
        Dir.mkdir(name)
        0
    end
end
def create_cmakefiles(path,name)
    fp = File.open(File.join(path,"CMakeLists.txt"),"w")
    fp<<'
# $Id:$
SET(target_CPP
)
SET(target_INCLUDE
)
SET(target_INCLUDE_DIR "")
# add components here
# end of additional components
INCLUDE_DIRECTORIES(${target_INCLUDE_DIR})
SET (target_SOURCES
${target_CPP}
${target_INCLUDE}
)

#ADD_LIBRARY('+name+' STATIC ${target_SOURCES})
SEGS_REGISTER_COMPONENT('+name+' "${target_INCLUDE_DIR}" "${target_CPP}" "${target_INCLUDE}" )'
    fp.close()
end
def update_parent_cmakefile(sub_dir,name)
    fp = File.open(File.join(sub_dir,"CMakeLists.txt"),"a+")
    fp<<"\nADD_SUBDIRECTORY(#{name})\n"
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
    Dir.mkdir_optional(File.join(sub_dir,name))
    Dir.mkdir_optional(File.join(sub_dir,name,"src"))
    Dir.mkdir_optional(File.join(sub_dir,name,"include"))
    Dir.mkdir_optional(File.join(sub_dir,name,"Docs"))
    update_parent_cmakefile(sub_dir,name)
    create_cmakefiles(File.join(sub_dir,name),name)
end
if(ARGV.size()!=2)
    show_usage()
else
    Dir.chdir("../Components")
    create_component(ARGV[0],ARGV[1])
    Dir.chdir("../Utilities")
end


