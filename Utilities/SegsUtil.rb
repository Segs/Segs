#!/usr/bin/env ruby
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
