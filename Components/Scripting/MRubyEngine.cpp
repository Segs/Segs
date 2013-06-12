#include <set>
#include <ace/Module.h>
#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/string.h>
#include <mruby/khash.h>
#include <mruby/dump.h>
#include <ace/Dirent.h>
#include <ace/Dirent_Selector.h>
#include <ace/DLL_Manager.h>
#include <ace/DLL.h>
#include <dirent.h>
#include <sstream>

#include "MRubyEngine.h"
static constexpr const char * initialization_code = {
    "module Kernel\n"
    "  def require(filename)\n"
    "    @@loaded_files ||= {}\n"
    "    return @@loaded_files[filename] if @@loaded_files.include?(filename)\n"
    "    @@loaded_files[filename] = nil # prevent infinite require loop\n"
    "    @@loaded_files[filename] = c_require(filename)\n"
    "  end\n"
    "end\n"
};

void split(const std::string &str,char split_by,std::vector<std::string> &result)
{
    std::stringstream ss(str);
    std::string item;
    while(std::getline(ss, item, split_by)) {
        result.push_back(item);
    }
}
std::string join(std::vector<std::string> &parts,char with)
{
    std::ostringstream ss;
    for(size_t i=0; i<parts.size(); ++i)
    {
        ss<<parts[i];
        if((i+1)!=parts.size())
            ss<<with;
    }
    return ss.str();
}
const char *FileLocator::traverse(ACE_Dirent *dir,const char *tgtfile)
{
    for (ACE_DIRENT *entry; (entry = dir->read ()) != 0; )
    {
        std::string entryname=entry->d_name;
        if( (entry->d_type==DT_DIR) && ((entryname==".") || (entryname=="..")) )
            continue;
        ACE_OS::strcmp(entry->d_name, tgtfile) == 0;
    }
}
bool FileLocator::file_exists(const char *fname)
{
    std::vector<std::string> elements;
    split(m_base_path,PATH_SEP,elements);
    split(fname,'/',elements); // ruby paths are always separated by '/'
    std::string full_path=join(elements,PATH_SEP);
    FILE *fp = fopen(full_path.c_str(),"r");
    if(fp)
    {
        fclose(fp);
        return true;
    }
    return false;
}

bool MRubyEngine::initialize()
{
    bool result=true;
    m_state = mrb_open();
    m_state->ud = this; //NOTICE: this is a hack, ud is mainly used in custom allocator mruby
    initialize_methods();
    m_ctx =  mrbc_context_new(m_state);
    m_ctx->capture_errors=1;
    mrb_load_string_cxt(m_state,initialization_code,m_ctx);
    if(m_state->exc) {
        mrb_p(m_state,mrb_obj_value(m_state->exc));
        result = false;
    }
    return result;
}
MRubyEngine::~MRubyEngine()
{
    if(m_state)
    {
        mrbc_context_free(m_state,m_ctx);
        m_ctx = nullptr;
        mrb_close(m_state);
        m_state = nullptr;
    }
}
void MRubyEngine::initialize_methods()
{
    mrb_define_method(m_state,m_state->kernel_module,"c_require",MRubyEngine::c_require,ARGS_REQ(1));
}

mrb_value MRubyEngine::c_require(mrb_state *state,mrb_value self)
{
    mrb_value fname;
    const char *file_name;
    if(mrb_get_args(state,"S",&fname))
        file_name = mrb_string_value_ptr(state,fname);
    else
        return mrb_nil_value();
    MRubyEngine *ctx  = (MRubyEngine *)state->ud;
    if(ctx->try_ruby_file(file_name))
        return mrb_true_value();
    if(ctx->try_shared_object(file_name))
        return mrb_true_value();
    return mrb_false_value();
}

bool    MRubyEngine::try_shared_object(const char *mod_name)
{
    // if a shared object file_name+".dll"/".so" exists, try to load it and call file_name+"_init"
    //TODO: Add mgems and mplugins to the search path here
    std::string init_symbol_name(mod_name);
    init_symbol_name="mrb_mruby_"+init_symbol_name+"_gem_init";
    ACE_DLL_Handle * dll = ACE_DLL_Manager::instance()->open_dll(init_symbol_name.c_str(),
                                                                 RTLD_LAZY,ACE_SHLIB_INVALID_HANDLE);
    if(dll && dll->symbol(init_symbol_name.c_str(),1)) {
        fInitFunc func = (fInitFunc)dll->symbol(init_symbol_name.c_str(),1);
        func(m_state);
        return true;
    }
    return false;
}
bool    MRubyEngine::try_ruby_file(const char *mod_name)
{
    //TODO: automatically compile the rb files, if out of date ?
    std::string rbc_extended_fname(mod_name);
    std::string rb_extended_fname(mod_name);
    rbc_extended_fname+=".rbc";
    // if a file_name+".rbc" load it, and evaluate the irep
    if(m_file_locator.file_exists(rbc_extended_fname.c_str())) {
        FILE *fp = fopen(rbc_extended_fname.c_str(),"rb");
        if(MRB_DUMP_OK==mrb_read_irep_file(m_state,fp))
            return true;
    }
    rb_extended_fname = std::string(mod_name) + ".rb";
    // if a file_name+".rb" exists, load it, parse and evaluate the irep
    if(m_file_locator.file_exists(rb_extended_fname.c_str())) {
        FILE *fp = fopen(rb_extended_fname.c_str(),"rb");
        //TODO: check compiled vs noncompiled dates ?
        mrb_load_file_cxt(m_state,fp,m_ctx);
        if(m_state->exc) {
            mrb_p(m_state,mrb_obj_value(m_state->exc));
            return false;
        }
        return true;
    }
}
class ScriptTypeError {

};
template<>
int fromRuby(mrb_state * mrb, mrb_value self) {
    mrb_value v = mrb_to_int(mrb,self);
    if(mrb->exc)
        throw ScriptTypeError();
    return mrb_fixnum(v);
}
