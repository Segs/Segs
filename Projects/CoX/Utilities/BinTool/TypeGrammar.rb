#!/usr/bin/env ruby
#require 'profile'
require 'rexml/document'
require 'pp'
require 'stringio'
require 'singleton'
include REXML
TEMPLATES={}
structures={}
filetypes={}
class TypeStorage
    include Singleton
    def initialize
        @types={}
    end
    def add_type(name,type)
        raise "type clash!" if @types[name]!=nil
        @types[name] = type
    end
    def get_type(name)
        @types[name]
    end
end
class Type
    
end
class EnumType < Type
    def initialize(name)
        @name=name
        @values={}
        TypeStorage.instance.add_type(name,self)
    end
    def add_value(val,name)
        @values[val]=name
    end
    def get_byte_size
        4
    end
    def init_val(into,name,offset,val)
        into.set_val(offset,CreatedField.new(offset,name,val))
    end

end
class BitfieldType < Type
    def initialize(name)
        @name=name
        @values={}
        TypeStorage.instance.add_type(name,self)
    end
    def add_value(val,name)
        @values[val]=name
    end
    def get_byte_size
        4
    end
    def read_from(stream)
        res=stream.read_int()        
        CreatedPrimitive.new(self,res)        
    end
    def init_val(into,name,offset,val)
        into.set_val(offset,CreatedField.new(offset,name,val))
    end
    def typename
        @name
    end
end
class PrimitiveType < Type
    def initialize(type_id)
        @type_id        = type_id
    end
    def read_from(bf)
        res=nil
        case(@type_id-3)
        when 0,3,4
            res=bf.read_pascal_str
        when 1,2,12
            res=bf.read_int()
        when 5
            raise "Unknown type !"
            return bf.read_bytes(128)
        when 6
            raise "Unknown type !"
            return bf.read_bytes(4)
        when 7
            res=bf.read_float()
        when 8
            #tgt_struct.set_val(offset,@bf.read_bytes(8)) # double ?
            res=[bf.read_float(),bf.read_float()]
        when 9
            #raise "Unknown type !"
            res=[bf.read_float(),bf.read_float(),bf.read_float()]
        when 10
            res=[bf.read_bytes(3).unpack("CCC")]
            bf.skip(1)
        when 11
            raise "Unknown type !"
            res=bf.read_bytes(2)
            bf.skip(2)
        when 13
            raise "Unknown type !"
            arr_size = bf.read_bytes(4)
            res=[]
            arr_size.times { res << @bf.read_bytes(4) }
        when 14
            raise "Unknown type !"
            arr_size = bf.read_bytes(4)
            res=[]
            arr_size.times {  res<<bf.read_bytes(4) }
        when 15
            arr_size = bf.read_int()
            res=[]
            arr_size.times { res << bf.read_pascal_str() }
        when 16
            ;
        when 17 # aligned bytes
            raise "Unknown type !"
            round_up_bytes = ((@param+3)&(~3))-@param
            val = bf.read_bytes(@param)
            bf.skip(round_up_bytes)
        end
        CreatedPrimitive.new(self,res)
    end
    def init_val(into,name,offset,val)
        into.set_val(offset,CreatedField.new(offset,name,val))
    end
    def serialize_out(visitor,val)
        visitor.visit_primitive(self,val)
    end
    def typename
        case @type_id
        when 12
            return "Vec3"
        when 13
            return "RGB"
        else
            return "some_type"
        end
    end
end
class StructTypeField
    attr_accessor :referenced_type,:offset,:init_to
    attr_reader   :name
    def initialize(name,type,flags,offset,init_to)
        @name,@referenced_type,@flags,@offset,@init_to = name,type,flags,offset.to_i(0),init_to
        if(@referenced_type.is_a?(PrimitiveType))
           @init_to= CreatedPrimitive.new(@referenced_type,init_to)
        end
    end
    def instantiate(tgt_struct)
        sub = StructArray.new((@flags&0x400)==0,@init_to,tgt_struct,@offset,@referenced_type.name)
        sub.init_storage()
        elem = sub.new_elem
        @referenced_type.bind_all_types()
        elem.init_from_tpl(@referenced_type)
        elem
    end
    def read_from(stream,tgt_struct,sub_size)
        @referenced_type.read_from(stream,tgt_struct,sub_size)
    end
    def get_flags
        @flags
    end
end
class StructureType < Type
    attr_reader :name
    @@recurse_count=0
    @@field_count=0
    def initialize(name)
        @bound = false # all TypeRef's were resolved ?
        @entries=[]
#        @name_val_map={}
        @name = name
        TypeStorage.instance.add_type(name,self)
    end
    def add_entry(name,type,flags,offset,init_to)
        raise "untyped value #{name}" if !type.kind_of?(Type)
        val = StructTypeField.new(name,type,flags,offset,init_to)
        @entries<<[name,val]
 #       @name_val_map[name]=val
    end
    def each_entry(&blck)
        bind_all_types() if !@bound
        raise "unresolved types !" if !@bound
        @entries.each {|e| yield e }
    end

    def bind_all_types
        all_bound=true
        @entries.each {|entr|
            v=entr[1]
            if v.referenced_type.is_a?(TypeRef)
                deref = v.referenced_type.dereference()
                raise "cannot find type!" if deref==nil
                all_bound=false if deref==nil
                v.referenced_type = deref
            end
        }
        @bound = all_bound
    end
    def each_non_compound(&blk)
        @entries.each {|entry|
            val = entry[1]
            offset   = val.offset
            next if((val.get_flags()&0x100)!=0) # skip if flag 0x100 is set
            next if val.referenced_type.is_a?(StructureType)
            yield entry
        }
    end
    def read_non_nested_structure(stream,tgt_struct)
        deb = false
        start = stream.bytes_left
        self.each_non_compound {|entry|
            name= entry[0]
            val = entry[1]
            offset   = val.offset
            read_val = val.referenced_type.read_from(stream)
            tgt_struct.set_val(offset,CreatedField.new(offset,name,read_val))
        }
        return start-stream.bytes_left
    end
    def read_from(stream,tgt_struct,sub_size)
        
        bytes_to_process=stream.read_int()
        sub_size-=4
        datasum=bytes_to_process
        if(bytes_to_process<=sub_size)
            bytes_to_process -= read_non_nested_structure(stream,tgt_struct)
        end
        if(bytes_to_process!=0)
            raise "Unprocessed bytes left #{bytes_to_process}!"
        end
        sub_size-=datasum
        # after entries
        while(sub_size>0)
            start                   = stream.bytes_left()
            entry_type,struct_size  = *stream.read_header()
            
            return 0 if(struct_size==0)

            raise "Wrong structure size!" if(struct_size>sub_size)

            sub_size -= start-stream.bytes_left()   # reduce number of bytes left by size of header

            # read structures
            return 0 if stream.bytes_left==0
            template_entry = nil
            @entries.each {|e|
                next if e[0]!=entry_type
                template_entry=e[1]
                break
            }
            raise "Couldn't find structure corresponding with #{entry_type}" if(!template_entry.referenced_type.is_a?(StructureType))
            
            offset = template_entry.offset
            field_flags = template_entry.get_flags()

            elem = template_entry.instantiate(tgt_struct)
            @@recurse_count+=1
            if @@recurse_count==-1
                pp tgt_struct
                exit()
            end
            #pp tgt_struct if entry_type == "Property"
            if template_entry.read_from(stream,elem,struct_size)==0
                raise "read_failed"
            end
            sub_size-=struct_size
        end
        return 1
    end
    def init_val(into,name,offset,val)
        into.set_val(offset,CreatedField.new(offset,name,nil))
        @@field_count+=1
    end
end
class CreatedPrimitive
    attr_reader :type,:value
    def initialize(type,value)
        @type,@value = type,value
    end
    def serialize_out(visitor)
        visitor.visit_primitive(self)
    end
end
class CreatedField
    attr_reader :name,:value,:offset
    def initialize(offset,name,value)
        @offset,@name,@value=offset,name,value
        raise "whoa" if value==0
    end
    def serialize_out(visitor)
        visitor.visit_field(self)
    end
end
class CreatedStructure
    attr_reader :typename
    def initialize(size,name)
        @typename=name
        @values=[]
    end
    def set_val(offset,value)
        #print "Setting vals at 0x"+offset.to_s(16).upcase+" -> #{value}\n"
        if(offset==0)
            @values[0] ||= {}
            @values[0][value.name] = value
        else
            @values[offset/4]=value
        end
    end
    def get_val(offset)
        @values[offset/4]
    end
    def init_from_tpl(tpl)
        tpl.each_entry {|entr|
            e = entr[1]
            e.referenced_type.init_val(self,e.name,e.offset,e.init_to)
        }
    end
    def serialize_out(visitor)
        sub_name = ""
        sub_name = @values[0][""].value.value if @values[0][""]!=nil
        visitor.enter_structure(self,sub_name)
        @values.each {|v|
            next if v==nil
            next if v.is_a?(Hash)
            if(v.is_a?(Array))
                v.each {|entry|
                    entry.serialize_out(visitor)
                }
            else
                v.serialize_out(visitor)
            end
        }
        visitor.leave_structure(self)
    end
end
class StructArray
    def initialize(type,entry_size,values,offset,type_name)
        @type       = type ? 2 : 1
        @struct     = values
        @entry_size = entry_size
        @my_offset  = offset
        @type_name  = type_name
        raise "typename cannot be empty" if type_name==nil
    end
    def init_storage
        @struct ||= CreatedStructure.new() if(@type!=1)
    end
    def new_elem()
        if(@type==1)
            return @struct
        else
            res = CreatedStructure.new(@entry_size,@type_name)
            if(@struct.get_val(@my_offset).is_a?(Array))
                @struct.get_val(@my_offset)<<res
            else
                @struct.set_val(@my_offset,[res])
            end
            return res
        end
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
class TypeRef < Type
    attr_reader :name,:type,:offset,:param,:sub_ref
    def initialize(type,sub_ref)
        raise "Typeref with nil type!" if type==nil
        @type,@sub_ref = type.to_i(0),sub_ref
    end
    def dereference()
        TypeStorage.instance.get_type(@sub_ref)
    end
end
class XMLWriter
    def initialize(tgt_stream)
        @indent=0
        @tgt_stream=tgt_stream
    end
    def indent()
        ("\t"*@indent)
    end
    def enter_structure(struct,name)
        @tgt_stream << indent() << "<struct type=\"#{struct.typename}\" sub_name=\"#{name}\">\n"
        @indent+=1
    end
    
    def leave_structure(struct)
        @tgt_stream << indent() << "</struct>\n"
        @indent-=1
    end
    def visit_field(field)
        if field.value!=nil && !(field.value.is_a?(CreatedPrimitive) && field.value.value=="")
            @tgt_stream << indent() << "<field name=\"#{field.name}\">\n"
            @indent+=1
            field.value.serialize_out(self)
            @indent-=1
            @tgt_stream << indent() << "</field>\n"
        else
            #printf((" " * (@indent+1) )+"<empty\\>\n")
        end
    end
    def visit_primitive(prim)
        val_s = ""
        if(prim.value.is_a?(Array))
            val_s = "["+prim.value.join(",")+"]"
        else
            val_s = prim.value.to_s()
        end
        @tgt_stream << indent() << "<value=\"#{val_s}\" type=\"#{prim.type.typename}\"\\>\n"
    end
    def visit_bitfield(bf)
    end
    def visit_enum()
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
    def read_float()
        read_bytes(4).unpack("F")[0]
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
    def read_header()
        entry_type    = read_pascal_str()
        num_entries    = read_int()
        p "FT: #{entry_type}" if num_entries==0
        return [entry_type,num_entries]
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
        
        @template = TypeStorage.instance.get_type(filetype)
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
    def read_data_blocks
        read_bin_version()
        res=@bf.read_header()
        return if (res[1]<=0)
        return if (res[0]!="Files1")
        @num_data_blocks = @bf.read_int()
        @data_blocks=[]
        p "Num of source files #{@num_data_blocks}"
        
        @num_data_blocks.times {
            @data_blocks<<DataBlock.new(@bf.read_pascal_str(),@bf.read_time())
        }
        #pp @data_blocks
    end
    def init_sub(templ,sub)
        
    end
    def serialize_from(file)
        @bf = BinFile.new(file)
        read_data_blocks()
        @bf.str_dump_info()
        res = CreatedStructure.new(0,file)
        res.init_from_tpl(@template)
        @template.read_from(@bf,res,@bf.bytes_left())
        serializer = XMLWriter.new(File.new("output.xml","w"))
        res.serialize_out(serializer)
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
    bf = BitfieldType.new(s.attributes['name'])
    s.each_element("field") {|fld|
        attr = fld.attributes
        bf.add_value(attr["mask"].to_i(0),attr["name"])
    }
    #p s    
}
doc.root.elements["enums"].each_element("enum") { |s|
    en = EnumType.new(s.attributes['name'])
    s.each_element("field") {|fld|
        attr = fld.attributes
        en.add_value(attr["value"].to_i(0),attr["name"])
    }
}
doc.root.elements["structures"].each_element("structure") { |s|
    strct = StructureType.new(s.attributes['name'])
    p s.attributes['name']
    s.each_element("type_ref") {|t_ref|
        attr = t_ref.attributes
        obj = TypeStorage.instance.get_type(attr['sub_ref'])
        t_flags = attr['type'].to_i(0) & 0xFF00 #>>8
        if(obj==nil)
            type_id = 0xFF & attr['type'].to_i(0)
            if(type_id==0x15 )#|| type_id==0x16 || type_id==0x13)
                obj=TypeRef.new(attr['type'],attr['sub_ref'])
            else
                obj=PrimitiveType.new(type_id)
            end
        end

        strct.add_entry(attr['name'],obj,t_flags,attr['offset'],attr['param'])
        
        }
}

doc.root.elements["filetypes"].each_element("filetype") { |s|

    strct = StructureType.new(s.attributes['name'])
    p s.attributes['name']
    s.each_element("type_ref") {|t_ref|
        attr = t_ref.attributes
        obj = TypeStorage.instance.get_type(attr['sub_ref'])
        type_id = 0xFF & attr['type'].to_i(0)
        t_flags = attr['type'].to_i(0) & 0xFF00 #>>8
        if(obj==nil)
            if(type_id==0x15 ) #|| type_id==0x16 || type_id==0x13)
                obj=TypeRef.new(attr['type'],attr['sub_ref'])
            else
                obj=PrimitiveType.new(type_id)
            end
        end
        strct.add_entry(attr['name'],obj,t_flags,attr['offset'],attr['param'])
        
        }
}
ss=Serializer.new('scenefile')
ss.serialize_from('City_00_01.bin')