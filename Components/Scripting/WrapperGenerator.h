#pragma once
#include <tuple>
#include <functional>
#include "mruby.h"
#include "mruby/data.h"
#include "mruby/value.h"
#include "mruby/class.h"
#include "mruby/variable.h"
#include "mruby/string.h"
#include "mruby/data.h"

#include "MRubyEngine.h"
struct ArgMapper {
    /*
        Ruby arg format chars
        o - Object
        S - String
        A - Array                           *
        H - Hash
        s - string 'char *,int'             *
        z - 0 terminated string 'char *'
        a - array [mrb_value *,mrb_int]
        f - float
        i - integer                         *
        b - binary  ?? boolean ?
        n - symbol
        & - block
        * - rest [mrb_value *,int]
        Arg modifiers:
        | - optional
*/
    template<typename T>
    static std::function< typename std::enable_if<std::is_function<T>::value, T>::type >
    make_function(T *t) {
      return { t };
    }
    template<typename R,class SELF,typename... Args>
    static void extract_args_and_call_method(mrb_state * mrb,SELF *self,R (SELF::*func)(Args...))
    {
        std::tuple<Args...> values;

        typedef typename make_indices<Args...>::type Indices;
        extract_args_tuple(mrb,Indices(),values);
        return forward_impl_method(Indices(), func,self,values);
    }
    template<typename R,typename... Args>
    static R extract_args_and_call(mrb_state * mrb,const std::function<R(Args...)> &func)
    {
        std::tuple<Args...> values;

        typedef typename make_indices<Args...>::type Indices;
        extract_args_tuple(mrb,Indices(),values);
        return forward_impl_call(Indices(), func,values);
    }
protected:
    template<size_t...> struct index_tuple{};

    template<size_t I, typename IndexTuple, typename... Types>
    struct make_indices_impl;

    template<size_t I, size_t... Indices, typename T, typename... Types>
    struct make_indices_impl<I, index_tuple<Indices...>, T, Types...>
    {
      typedef typename
        make_indices_impl<I + 1,
                          index_tuple<Indices..., I>,
                          Types...>::type type;
    };

    template<size_t I, size_t... Indices>
    struct make_indices_impl<I, index_tuple<Indices...> >
    {
      typedef index_tuple<Indices...> type;
    };

    template<typename... Types>
    struct make_indices
      : make_indices_impl<0, index_tuple<>, Types...>
    {};
    // this function's role is to extract the parameters from ruby given list of typed references
    template <size_t... Indices, class... Args, class SELF,class Ret>
    static Ret forward_impl_method(index_tuple<Indices...>,
                                   Ret (SELF::*func)(Args...),
                                   SELF *self,
                                   std::tuple<Args...> tuple
                                   )
    {
        return (self->*func)(std::get<Indices>(tuple)...);
    }
    template <size_t... Indices, class... Args, class Ret>
    static Ret forward_impl_call(index_tuple<Indices...>,
                                   const std::function<Ret(Args...)> &func,
                                   std::tuple<Args...> tuple
                                   )
    {
        return func(std::get<Indices>(tuple)...);
    }
    template<size_t ...indices, typename... Args>
    static void extract_args_tuple(mrb_state * mrb, index_tuple<indices...>,std::tuple<Args... > &args)
    {
        extract_args(mrb,std::get<indices>(args)...);
    }
    static void extract_args(mrb_state * mrb)
    {
    }
    template<typename T, typename... Args>
    static void extract_args(mrb_state * mrb,T &value, Args... args)
    {
        extract_arg(mrb,value);
        extract_args(mrb,args...);
    }
    static void extract_arg(mrb_state * mrb,char *& tgt)
    {
        // get the c++ version of the args out of the arguments
        mrb_value arg0;
        mrb_get_args(mrb, "S", &arg0);
        // call the C++ method with the correct arguments
        tgt=mrb_str_ptr(arg0)->ptr;
    }
    static void extract_arg(mrb_state * mrb,const char *& tgt)
    {
        // get the c++ version of the args out of the arguments
        mrb_value arg0;
        mrb_get_args(mrb, "S", &arg0);
        // call the C++ method with the correct arguments
        tgt=mrb_str_ptr(arg0)->ptr;
    }
    static void extract_arg(mrb_state * mrb,const uint8_t *& tgt)
    {
        // get the c++ version of the args out of the arguments
        mrb_value arg0;
        mrb_get_args(mrb, "S", &arg0);
        // call the C++ method with the correct arguments
        tgt=(uint8_t *)mrb_str_ptr(arg0)->ptr;
    }
    static void extract_arg(mrb_state * mrb,std::string& tgt)
    {
        // get the c++ version of the args out of the arguments
        char* arg0;
        int  len;
        mrb_get_args(mrb, "s", &arg0,&len);
        tgt = std::string(arg0,len);
    }
    static void extract_arg(mrb_state * mrb,int & tgt)
    {
        mrb_get_args(mrb, "i", &tgt);
    }
    static void extract_arg(mrb_state * mrb,size_t & tgt)
    {
        int value;
        mrb_get_args(mrb, "i", &value);
        tgt=value;
    }
    static void extract_arg(mrb_state * mrb,void *& tgt)
    {
        mrb_value value;
        mrb_get_args(mrb, "o", &value);
        tgt=mrb_voidp(value);
    }
    static void extract_arg(mrb_state * mrb,const void *& tgt)
    {
        mrb_value value;
        mrb_get_args(mrb, "o", &value);
        tgt=mrb_voidp(value);
    }
};
// The c struct that mrb manages, with just a pointer to our c++ class
template<class TYPE>
struct TypeBinding  {
    TYPE * m_p = nullptr;
    static mrb_value initialize(mrb_state * mrb, mrb_value self) {
        /* search the incomming value for a new binding struct */
        auto p = (TypeBinding<TYPE> *)mrb_get_datatype(mrb, self, &binding);
        /* if it already has one, get rid of it */
        if (p)
            free_wrap(mrb, p);
        /* grab a new struct from the mrb managed memory pool */
        p = (TypeBinding<TYPE> *)mrb_malloc(mrb, sizeof(TypeBinding<TYPE>));
        /* new up our c++ class on this struct */
        p->m_p = wrapped_constructor(mrb,self);
        /* set the outgoing value with the correct data pointers */
        DATA_PTR(self) = p;
        DATA_TYPE(self) = &binding;
        return self;
    }
    static void free_wrap(mrb_state * mrb, void * ptr) {
        auto p = (TypeBinding<TYPE> *)ptr;
            if (p && p->m_p){
                delete p->m_p;
                p->m_p = nullptr;
            }
            mrb_free(mrb, ptr);
    }
    static TYPE *fromRuby(mrb_state * mrb, mrb_value self) {
        TypeBinding<TYPE> * p = (TypeBinding<TYPE> *)mrb_get_datatype(mrb, self, &binding);
        if( p && p->m_p)
            return p->m_p;
        return nullptr;
    }
    static mrb_value toRuby(mrb_state * mrb, TYPE *self) {
        return mrb_nil_value();
    }
    static TYPE *wrapped_constructor(mrb_state * mrb, mrb_value self) {
        //ArgMapper::extract_args_and_call_method
    }
    // A struct that configures the cleanup method on our data
    static constexpr mrb_data_type binding = { TYPE::name, free_wrap };
};
template<class T>
constexpr mrb_data_type TypeBinding<T>::binding;

template<typename TYPE>
TYPE fromRuby(mrb_state * mrb, mrb_value self);

template<typename TYPE>
mrb_value toRuby(mrb_state * mrb, TYPE self);


#define WRAP_FUNCTION(name) mrb_value wrap##_##name(mrb_state * mrb,mrb_value self) \
{\
    ArgMapper::extract_args_and_call(mrb,ArgMapper::make_function(name));\
    return self;\
}
#define WRAP_METHOD(type,method) mrb_value wrap##_##type##_##method(mrb_state * mrb,mrb_value self) \
{\
    auto p = TypeBinding<type>::fromRuby(mrb,self);\
    if (!p)\
        return self;\
    ArgMapper::extract_args_and_call_method(mrb,p,&type::method);\
    return self;\
}
