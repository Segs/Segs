#!/usr/bin/env ruby
require 'pp'
class CMakeFile
    attr_reader :current_path
    def initialize(path,parent=nil)
        @parent=parent
        @current_path=path
        if(path[0]!='.' && parent!=nil) # path relative to parent
            @current_path = File.join(parent.current_path,path)
        end
    end
    def create_component(name)
        p @current_path
        raise "CMakeLists.txt already exists" if File::exists?(File.join(@current_path,"CMakeLists.txt"))
        fp = File.open(File.join(@current_path,"CMakeLists.txt"),"w")
        fp<<'
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
'
        fp<<"#ADD_LIBRARY(#{name} STATIC ${target_SOURCES})\n"
        fp<<"SEGS_REGISTER_COMPONENT(#{name} \"${target_INCLUDE_DIR}\" \"${target_CPP}\" \"${target_INCLUDE}\" )\n"
        fp.close()
    end
    def parent_dir(path) # returns absolute path of given path
        File.expand_path((File.join(path,"..")))
    end
    # name is the same as the subdir name is the same as the component name
    def add_subdir(name,prefix="",optional=false)
        build_name = name
        build_name = prefix+"_"+name if !prefix.nil? && prefix.size>0
        
        fp = File.open(File.join(@current_path,"CMakeLists.txt"),"a+")
        fp <<"\n"
        if(optional)
            # add option
            fp << "SET(BUILD_#{build_name} TRUE CACHE BOOL \"Build #{name}?\")\n"
        end
        fp<<"IF(BUILD_#{build_name})\n\t" if(optional)
        fp<<"ADD_SUBDIRECTORY(#{name})\n"
        fp<<"ENDIF(BUILD_#{build_name})" if(optional)
    end
end
