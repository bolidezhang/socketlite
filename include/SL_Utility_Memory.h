#ifndef SOCKETLITE_UTILITY_MEMORY_H
#define SOCKETLITE_UTILITY_MEMORY_H
#include <cstring>
#include "SL_Config.h"

class SL_Utility_Memory
{
public:
    SL_Utility_Memory()
    {
    }

    ~SL_Utility_Memory()
    {
    }

    //内存拷贝
    //经测试:  window7-32 vc2010 cpu: 4*i5-2310 memory: 4G:
    //              1)小块内存时, 按8字节整数赋值,性能提升明显;
    //              2)大块内存时, 按8字节整数赋值,性能反而下降;所以内存块大于256时,直接调标准库memcpy.
    //         linux:
    //              1)性能可能好些,也可能差些(使用前,请在运行环境下仔细测量)
    //              2)centos6.3      x86-64 kernel:2.6.32 gcc:4.4.6 cpu:4*i5-2310  memory:4G 8字节整数赋值 性能提升明显
    //              3)centos7.0.1406 x86-64 kernel:3.10.0 gcc:4.8.2 cpu:4*i5-3330S memory:8G 8字节整数赋值 比 标准memcpy 性能略差.
    static void* memcpy(void *dest, const void *src, size_t n);

    //内存清零
    static void memclear(void *dest, size_t n);

    //内存比较
    static inline int memcmp(const void *s1, const void *s2, size_t n)
    {
        return std::memcmp(s1, s2, n);
    }

    //内存置值
    //另一种实现方式: 预先计算  8字节整数赋值(每字节都等于整数c的数值)
    //                        4字节整数赋值(每字节都等于整数c的数值)
    //                        2字节整数赋值(每字节都等于整数c的数值)
    static inline void* memset(void *dest, int c, size_t n)
    {
        return std::memset(dest, c, n);
    }

    static inline void* memmove(void *dest, const void *src, size_t n)
    {
        return std::memmove(dest, src, n);
    }
};

#ifdef SOCKETLITE_OS_WINDOWS
    #define SL_MEMCPY(dest, src, n)     std::memcpy(dest, src, n)
    #define SL_MEMCLEAR(dest, n)        std::memset(dest, 0, n)
    #define SL_MEMCMP(s1, s2, n)        std::memcmp(s1, s2, n)
    #define SL_MEMSET(dest, c, n)       std::memset(dest, c, n)
    #define SL_MEMMOVE(dest, src, n)    std::memmove(dest, src, n)

    #define sl_memcpy(dest, src, n)     SL_Utility_Memory::memcpy(dest, src, n)
    #define sl_memclear(dest, n)        SL_Utility_Memory::memclear(dest, n)
    #define sl_memcmp(s1, s2, n)        std::memcmp(s1, s2, n)
    #define sl_memset(dest, c, n)       std::memset(dest, c, n)
    #define sl_memmove(dest, src, n)    std::memmove(dest, src, n)
#else
    #define SL_MEMCPY(dest, src, n)     std::memcpy(dest, src, n)
    #define SL_MEMCLEAR(dest, n)        std::memset(dest, 0, n)
    #define SL_MEMCMP(s1, s2, n)        std::memcmp(s1, s2, n)
    #define SL_MEMSET(dest, c, n)       std::memset(dest, c, n)
    #define SL_MEMMOVE(dest, src, n)    std::memmove(dest, src, n)

    //因SL_Utility_Memory::memcpy在ios下可能会崩毁,
    //所以在非windows下需要定义宏SOCKETLITE_USE_CUSTOM_MEMCPY来启用sl_memcpy实现为SL_Utility_Memory::memcpy
    #ifdef SOCKETLITE_USE_CUSTOM_MEMCPY
        #define sl_memcpy(dest, src, n) SL_Utility_Memory::memcpy(dest, src, n)
        #define sl_memclear(dest, n)    SL_Utility_Memory::memclear(dest, n)
    #else
        #define sl_memcpy(dest, src, n) std::memcpy(dest, src, n)
        #define sl_memclear(dest, n)    std::memset(dest, 0, n)
    #endif

    #define sl_memcmp(s1, s2, n)        std::memcmp(s1, s2, n)
    #define sl_memset(dest, c, n)       std::memset(dest, c, n)
    #define sl_memmove(dest, src, n)    std::memmove(dest, src, n)
#endif

#endif

