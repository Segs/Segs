#pragma once
/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#include <mruby.h>
#include <string>
#include <ace/Dirent.h>
struct mrbc_context;
//TODO: Move FileLocator into it's own component -> consider more fleshed-out Filesystem Layer ?
class FileLocator
{
#ifdef WIN32
#   define PATH_SEP '\\'
#else
#   define PATH_SEP '/'
#endif
    std::string m_base_path;
    const char *traverse(ACE_Dirent *dir,const char *tgtfile);
public:
    FileLocator(const char *base_path) : m_base_path(base_path)
    {}
    bool file_exists(const char *fname);
};
class MRubyEngine
{
typedef void (*fInitFunc)(mrb_state *);
        FileLocator m_file_locator;
        mrbc_context* m_ctx;
public:
        mrb_state *m_state;
        static constexpr const char * initialization_code = {
            "class Kernel\n"
            "  def require(filename)\n"
            "    @@loaded_files ||= {}\n"
            "    return @@loaded_files[filename] if @@loaded_files.include?(filename)\n"
            "    @@loaded_files[filename] = nil # prevent infinite require loop\n"
            "    @@loaded_files[filename] = c_require(filename)\n"
            "  end\n"
            "end\n"
        };
                MRubyEngine() : m_file_locator(".") {}
        void    initialize();
        void    initialize_methods();
static mrb_value c_require(mrb_state *state,mrb_value self);
private:
        bool    try_shared_object(const char *mod_name);
        bool    try_ruby_file(const char *mod_name);
};
