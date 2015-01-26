#ifndef SOCKETLITE_MALLOC_H
#define SOCKETLITE_MALLOC_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
    #pragma once
#endif
#include <stdlib.h>
#include <string.h>
#include "SL_Config.h"

//比较优秀malloc的实现
//memory allocation: malloc implement
//1.jemalloc freebsd/facebook/firefox
//#define SOCKETLITE_MALLOC_JEMALLOC
//2.google tcmalloc
//#define SOCKETLITE_MALLOC_TCMALLOC    
//3.hoard - The Hoard Memory Allocator
//#define SOCKETLITE_MALLOC_HOARD
//4.nedmalloc - ned Productions dlmalloc
//#define SOCKETLITE_MALLOC_NEDMALLOC
//5.Lockless malloc - Lockless Inc. Low level software to optimize performance
//#define SOCKETLITE_MALLOC_LOCKLESS
//6.intel tbbmalloc
//#define SOCKETLITE_MALLOC_TBBMALLOC
//7.ptmalloc - a) linux glibc default malloc b) 起源于 Doug Lea 的 dlmalloc
//#define SOCKETLITE_MALLOC_PTMALLOC
//8.dlmalloc - Doug Lea dlmalloc
//#define SOCKETLITEZ_MALLOC_DLMALLOC

#if defined(SOCKETLITE_MALLOC_JEMALLOC)
    #define JEMALLOC_NO_DEMANGLE
    #include <jemalloc/jemalloc.h>

    //重新定义内存分配器
    #define sl_malloc(size) je_malloc(size)
    #define sl_calloc(count, size) je_calloc(count, size)
    #define sl_realloc(ptr, size) je_realloc(ptr, size)
    #define sl_free(ptr) je_free(ptr)

    //替换运行时库的默认内存分配器
    #define malloc(size) je_malloc(size)
    #define calloc(count, size) je_calloc(count, size)
    #define realloc(ptr, size) je_realloc(ptr, size)
    #define free(ptr) je_free(ptr)
#elif defined(SOCKETLITE_MALLOC_TCMALLOC)
    #include <google/tcmalloc.h>

    //重新定义内存分配器
    #define sl_malloc(size) tc_malloc(size)
    #define sl_calloc(count, size) tc_calloc(count, size)
    #define sl_realloc(ptr, size) tc_realloc(ptr, size)
    #define sl_free(ptr) tc_free(ptr)

    //替换运行时库的默认内存分配器
    #define malloc(size) tc_malloc(size)
    #define calloc(count, size) tc_calloc(count, size)
    #define realloc(ptr, size) tc_realloc(ptr, size)
    #define free(ptr) tc_free(ptr)
#else
    //重新定义内存分配器
    #define sl_malloc(size) ::malloc(size)
    #define sl_calloc(count, size) ::calloc(count, size)
    #define sl_realloc(ptr, size) ::realloc(ptr, size)
    #define sl_free(ptr) ::free(ptr)
#endif

//默认的内存块大小，经验值
#define SL_DEFAULT_MEMORY_BLOCK_SIZE    8192

class SL_Alloc_New
{
public:
    static void* allocate(size_t n)
    { 
        return new char[n];
    }

    static void deallocate(void *p)
    { 
        delete [](char *)p;
    }

    static void* reallocate(void *p, size_t n)
    {
        return NULL;
    }

};

class SL_Alloc_Malloc
{
public:
    static void* allocate(size_t n)
    { 
        return sl_malloc(n);
    }

    static void deallocate(void *p)
    { 
        sl_free(p);
    }

    static void* reallocate(void *p, size_t n)
    {
        return sl_realloc(p, n);
    }

};

#ifdef SL_ALLOC_NEW
    #define SL_ALLOC    SL_Alloc_New
#else
    #define SL_ALLOC    SL_Alloc_Malloc
#endif

template <typename T>
class SL_Allocator
{
public:
    typedef T           value_type;
    typedef T*          pointer;
    typedef const T*    const_pointer;
    typedef T&          reference;
    typedef const T&    const_reference;
    typedef size_t      size_type;

    virtual int clear()
    {
        return 0;
    }

    virtual void* allocate(size_type n)
    { 
        return SL_ALLOC::allocate(n);
    }

    virtual void deallocate(void *p)
    { 
        SL_ALLOC::deallocate(p);
    }
    
    virtual pointer construct()
    {
        void *p = SL_ALLOC::allocate(sizeof(T));
        pointer object = new (p) value_type();
        return object;
    }

    virtual void construct(T *p)
    {
        new (p) value_type();
    }

    virtual void destroy(T *p)
    {
        ((pointer)p)->~T();
        deallocate(p);
    }

    static SL_Allocator* Instance()
    {
        return &m_Alloc;
    }

private:
    static SL_Allocator<char> m_Alloc;
};

#endif

