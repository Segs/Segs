#!/usr/bin/env ruby
require 'rexml/document'
require 'pp'
require 'stringio'
include REXML
TEMPLATES={}
structures={}
filetypes={}

class StructureTemplate
    attr_reader :name
    def initialize(name)
        @name=name
        @entries=[]
    end
    def add_entry(entry)
        @entries<<entry
    end
    def to_c_code
        
    end
    def [](str)
        @entries.each {|e|
            return e if e.name==str    
        }
        return nil
    end
    def each_entry(&blck)
        @entries.each {|e| yield e }
    end
end
class CreatedStructure
    def initialize(size=0,name="")
        @typename=name
        @values=[]
    end
    def set_val(name,offset,value)
        #print "Setting vals at 0x"+offset.to_s(16).upcase+" -> #{value}\n"
        @values[offset/4]=[name,value]
    end
    def push_back(value)
        @values<<value
    end
    def init_from_tpl(tpl)
        tpl.each_entry {|e|
            offset = e.offset
            case((e.type&0xFF)-5)
            when 0,10
                set_val(e.name,offset,e.param)
            when 1,2 # string pointer
                set_val(e.name,offset,e.param) if(e.param.is_a?(String)&&e.param.size>0)                    
            when 3 # string as an array
                if(e.param.is_a?(String)&&e.param.size>0)
                    start_idx=0
                    e.param.each_char {|c|
                        set_val(e.name,offset+start_idx,c)
                        start_idx+=1
                    }
                end
            when 4
                set_val(e.name,offset,e.param)
            when 5 # float
                set_val(e.name,offset,e.param)
            end
        }
    end
end
class StructArray
    def initialize(type,entry_size,values,offset,type_name)
        @type       = type ? 2 : 1
        @struct     = values
        @entry_size = entry_size
        @my_offset  = offset
        @type_name  =type_name
    end
    def init_storage
        if(@type!=1)
            @struct ||= CreatedStructure.new()
        end
    end
    def new_elem()
        if(@type==1)
            return @struct
        else
            res = CreatedStructure.new(@entry_size,@type_name)
            @struct.push_back(res)
            return res
        end
    end
    def set_val(name,offset,value)
        @struct.set_val(name,@my_offset+offset,value)
    end
end
class BitFieldTemplate
    def initialize(name)
    end
    def to_c_code
        
    end
    def encode(flags)
    end
    def decode(number)
    end
end
class EnumTemplate
    def initialize(name)
    end
    def to_c_code
        
    end
    def encode(flags)
    end
    def decode(number)
    end
end
class TypeRef
    attr_reader :name,:type,:offset,:param,:sub_ref
    def initialize(name,type,offset,param,sub_ref)
        @name,@type,@offset,@param,@sub_ref = name,type.to_i(0),offset.to_i(0),param,sub_ref
    end
    def each_entry(&blck)
        TEMPLATES[@sub_ref].each_entry {|e| yield e }
    end
    def is_structure_ref
        @type==0x15 || @type==0x16 || @type==0x13
    end

end
class BinFile
    def initialize(filename)
        @deb_cnt = 0
        @stream = StringIO.new(File.new(filename,"rb").read())
    end
    def read_bytes(num_bytes)
        @stream.read(num_bytes)
    end
    def read_int()
        read_bytes(4).unpack("I")[0]
    end
    def read_short()
        read_bytes(2).unpack("S")[0]
    end
    def read_time()
        Time.at(read_int().to_i)
    end
    def read_pascal_str()
        
        strlen = read_short
        res = read_bytes(strlen)
        unalign = @stream.tell()&3
        if(unalign!=0)
            @deb_cnt+=1
            @stream.seek(4-unalign,IO::SEEK_CUR)
        end
        res
    end
    def str_dump_info
            p @stream.tell()
    end
    def bytes_left
        @stream.size()-@stream.tell()
    end
    def skip(num)
        @stream.seek(num,IO::SEEK_CUR)
    end
end
class DataBlock
    def initialize(blok_name,unk1)
        @name = blok_name
        @mod_time = unk1
    end
end
class Serializer
    def initialize(filetype)
        @template=TEMPLATES[filetype]
        @bf = nil
        @deb_count=0
    end
    def read_bin_version()
        magic=@bf.read_bytes(8)
        raise "Wrong file type" if magic!="CrypticS"
        @crc = @bf.read_int()
        @parse_type=@bf.read_pascal_str()
        p "PT: #{@parse_type}"
    end
    def read_header()
        @entry_type    = @bf.read_pascal_str()
        num_entries    = @bf.read_int()
        p "FT: #{@entry_type}" if num_entries==0
        return num_entries
    end
    def read_data_blocks
        read_bin_version()
        return if (read_header()<=0)
        return if (@entry_type!="Files1")
        @num_data_blocks = @bf.read_int()
        @data_blocks=[]
        p "Num of source files #{@num_data_blocks}"
        
        @num_data_blocks.times {
            @data_blocks<<DataBlock.new(@bf.read_pascal_str(),@bf.read_time())
        }
        pp @data_blocks
    end
    def read_non_nested_structure(current_template,tgt_struct)
        deb = false
        start = @bf.bytes_left
        current_template.each_entry {|entry|
            next if((entry.type&0x100)!=0) # skip if flag 0x100 is set
    
            offset = entry.offset
            fixed_type=(entry.type&0xFF)-3
            
            case(fixed_type)
            when 0,3,4
                tgt_struct.set_val(entry.name,offset,@bf.read_pascal_str)
            when 1,2,12
                tgt_struct.set_val(entry.name,offset,@bf.read_int())
            when 5
                raise "Unknown type !"
                tgt_struct.set_val(entry.name,offset,@bf.read_bytes(128))
            when 6
                tgt_struct.set_val(entry.name,offset,@bf.read_bytes(4))
            when 7
                tgt_struct.set_val(entry.name,offset,@bf.read_bytes(4).unpack("F")[0])
            when 8
                #tgt_struct.set_val(offset,@bf.read_bytes(8)) # double ?
                val = [@bf.read_bytes(4).unpack("F")[0],@bf.read_bytes(4).unpack("F")[0]]
                tgt_struct.set_val(entry.name,offset,val)
                tgt_struct.set_val(entry.name,offset+4,:prev)
            when 9
                raise "Unknown type !"
                tgt_struct.set_val(entry.name,offset,@bf.read_bytes(12)) # vec3 ?
            when 10
                tgt_struct.set_val(entry.name,offset,[:rgb,@bf.read_bytes(3).unpack("CCC")])
                @bf.skip(1)
            when 11
                raise "Unknown type !"
                tgt_struct.set_val(entry.name,offset,@bf.read_bytes(2))
                @bf.skip(2)
            when 13
                raise "Unknown type !"
                arr_size = @bf.read_bytes(4)
                arr=[]
                arr_size.times { arr << @bf.read_bytes(4) }
                tgt_struct.set_val(entry.name,offset,arr)
            when 14
                raise "Unknown type !"
                arr_size = @bf.read_bytes(4)
                arr=[]
                arr_size.times {  arr<<@bf.read_bytes(4) }
                tgt_struct.set_val(entry.name,offset,arr)
            when 15
                arr_size = @bf.read_bytes(4)
                arr=[]
                arr_size.times { arr << @bf.read_pascal_str() }
                tgt_struct.set_val(entry.name,offset,arr)
            when 16
                ;
            when 17 # aligned bytes
                raise "Unknown type !"
                round_up_bytes = ((entry.param+3)&(~3))-entry.param
                tgt_struct.set_val(entry.name,offset,@bf.read_bytes(entry.sub_size))
                @bf.skip(round_up_bytes)
            end
        }
        return start-@bf.bytes_left
    end
    def read_structured_data(current_template,tgt_struct,sub_size)
        
        raise "Must get Structure template!" if !current_template.is_a?(StructureTemplate)
        bytes_to_process=@bf.read_int()
        sub_size-=4
        datasum=bytes_to_process
        if(bytes_to_process<=sub_size)
            count=0
            bytes_to_process -= read_non_nested_structure(current_template,tgt_struct)
            #pp tgt_struct
        end
        if(bytes_to_process!=0)
            raise "Unprocessed bytes left #{bytes_to_process}!"
        end
        sub_size-=datasum
        # after entries
        while(sub_size>0)
            start = @bf.bytes_left()
            struct_size = read_header()
            return 0 if(struct_size==0)

            raise "Wrong structure size!" if(struct_size>sub_size)

            sub_size -= start-@bf.bytes_left()   # reduce number of bytes left by size of header
            
            #p ("Parsing sub of size #{struct_size} of type #{@entry_type}")
            # read structures
            return 0 if @bf.bytes_left==0
            template_entry = current_template[@entry_type]
            p template_entry if @entry_type==nil
            #p template_entry
            raise "Couldn't find structure corresponding with #{@entry_type}" if(template_entry==nil)||!template_entry.is_structure_ref()
            offset = template_entry.offset

            case template_entry.type&0xFF
            when 0x15 # substructure
                sub = StructArray.new((template_entry.type&0x400)==0,template_entry.param.to_i(0),tgt_struct,offset,template_entry.name)
                sub.init_storage()
                elem = sub.new_elem
                elem.init_from_tpl(template_entry)
                return 0 if(0==read_structured_data(TEMPLATES[template_entry.sub_ref],elem,struct_size))
                    
                @deb_count+=1
                if(@deb_count==20)
                    #exit()
                end
                
            when 0x13
                raise "whoa"
            when 0x16
                raise "whoa"
            else
                raise "whoa"
            end
            sub_size-=struct_size
        end
        return 1
    end
    def init_sub(templ,sub)
        
    end
    def serialize_from(file)
        @bf = BinFile.new(file)
        read_data_blocks()
        @bf.str_dump_info()
        res = CreatedStructure.new
        read_structured_data(@template,res,@bf.bytes_left())
        pp res
        exit()
        #@header=CommonHeader.new()
        #@template
    end
    def serialize_to(structures,file)
    end
end


file = File.new("templates.xml")
doc = Document.new(file)
doc.root.elements["bitfields"].each_element("bitfield") { |s|
    p s    
}
doc.root.elements["enums"].each_element("enum") { |s|
    p s    
}

doc.root.elements["structures"].each_element("structure") { |s|
    strct = StructureTemplate.new(s.attributes['name'])
    TEMPLATES[s.attributes['name']]=strct
    s.each_element("type_ref") {|t_ref|
        p t_ref
        attr = t_ref.attributes
        obj=TypeRef.new(attr['name'],attr['type'],attr['offset'],attr['param'],attr['sub_ref'])
        strct.add_entry(obj)
        }
}
doc.root.elements["filetypes"].each_element("filetype") { |s|
    p s
    strct = StructureTemplate.new(s.attributes['name'])
    TEMPLATES[s.attributes['name']]=strct
    s.each_element("type_ref") {|t_ref|
        p t_ref
        attr = t_ref.attributes
        obj=TypeRef.new(attr['name'],attr['type'],attr['offset'],attr['param'],attr['sub_ref'])
        strct.add_entry(obj)
        }
    
}

ss=Serializer.new('tricks')
ss.serialize_from('tricks.bin')