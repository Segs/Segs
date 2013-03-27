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
#include "WrapperGenerator.h"
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
class Object {

};
class Class {
    mrb_state *m_state;
public:
    Class(mrb_state *s,RClass *v) : m_rb_class(v) {}
    template<typename F>
    Class &define_method(const char *name,F f) {
        //TODO: create proper argc
        mrb_define_class_method(m_state,m_rb_class,name,f,0);
        return *this;
    }
    template<typename T>
    Class &define_constructor()
    {
        //mrb_define_singleton_method(m_state,m_rb_class,"initialize",&TypeBinding<T>::initialize,0);
        return *this;
    }
    void fin(){}
protected:
    RClass *m_rb_class;
};

class MRubyEngine
{
typedef void (*fInitFunc)(mrb_state *);
        FileLocator m_file_locator;
        mrbc_context* m_ctx;
public:
        mrb_state *m_state;
                MRubyEngine() : m_file_locator(".") {}
        void    initialize();
        void    initialize_methods();

        template<typename T>
        Class   define_class(RClass * superclass=nullptr)
        {
            if(nullptr==superclass)
                superclass=m_state->object_class;
            auto v = mrb_define_class(m_state,T::name, superclass);
            return Class(m_state,v);
        }

static mrb_value c_require(mrb_state *state,mrb_value self);
private:
        bool    try_shared_object(const char *mod_name);
        bool    try_ruby_file(const char *mod_name);
};
