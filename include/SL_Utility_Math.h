#ifndef SOCKETLITE_UTILITY_MATH_H
#define SOCKETLITE_UTILITY_MATH_H
#include <math.h>
#include "SL_Config.h"

//因整数取模(%)操作性能不高,所以特别定义此宏
//经测试: 1)windows, 性能提升显著 2)linux，提升不大
#define SL_MOD(x, y) ((x)-(x)/(y)*(y))

class SL_Utility_Math
{
public:
    SL_Utility_Math()
    {
    }

    ~SL_Utility_Math()
    {
    }

    //计算以2为底的整数的对数值
    //经测试: 1)windows, 方法1 比 方法2 性能高
    //        2)linux,   方法2 比 方法1 性能高
    static inline int log2(uint val)
    {        
        #ifdef SOCKETLITE_OS_WINDOWS
                //方法1
                int result = 0;
                while ((val >>= 1) != 0)
                {
                    ++result;
                }
                return result;
        #else
                //方法2
                if (val < 2)
                    return 0;
                if (val < 4)
                    return 1;
                if (val < 8)
                    return 2;
                if (val < 16)
                    return 3;
                if (val < 32)
                    return 4;
                if (val < 64)
                    return 5;
                if (val < 128)
                    return 6;
                if (val < 256)
                    return 7;
                if (val < 512)
                    return 8;
                if (val < 1024)
                    return 9;
                if (val < 2048)
                    return 10;
                if (val < 4096)
                    return 11;
                if (val < 8192)
                    return 12;
                if (val < 16384)
                    return 13;
                if (val < 32768)
                    return 14;
                if (val < 65536)
                    return 15;
                if (val < 131072)
                    return 16;
                if (val < 262144)
                    return 17;
                if (val < 524288)
                    return 18;
                if (val < 1048576)
                    return 19;
                if (val < 2097152)
                    return 20;
                if (val < 4194304)
                    return 21;
                if (val < 8388608)
                    return 22;
                if (val < 16777216)
                    return 23;
                if (val < 33554432)
                    return 24;
                if (val < 67108864)
                    return 25;
                if (val < 134217728)
                    return 26;
                if (val < 268435456)
                    return 27;
                if (val < 536870912)
                    return 28;
                if (val < 1073741824)
                    return 29;
                if (val < 2147483648)
                    return 30;

                return 31;
        #endif
    }
};

#endif

