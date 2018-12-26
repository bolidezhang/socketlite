#ifndef SOCKETLITE_CRYPTO_RAND_H
#define SOCKETLITE_CRYPTO_RAND_H
#include "SL_Config.h"

class SL_Crypto_Rand
{
public:
    enum TYPE
    {
        RANDOM_CHAR = 0,        // 按unsinged char 随机生成内存块
        VISIBLE_CHAR,           // 按char 随机生成 可显字符串(键盘可以输入)
        STANDARD_CHAR,          // 按char 随机生成 标准字符串(不包括特殊字符串)
        ENLARGE_CHAR,           // 按char 随机生成 增强型标准字符串(包括自定义的特殊字符)
        NUMBER_CHAR,            // 按char 随机生成 纯数字(0-9)
        LOWERCASE_CHAR,         // 按char 随机生成 小写字母(a-z)
        UPPERCASE_CHAR,         // 按char 随机生成 大写字母(A-Z)

        TYPE_NUMS,
    };

    SL_Crypto_Rand();
    virtual ~SL_Crypto_Rand();

    virtual void    seed(int nseed) = 0;
    virtual int     random_int() = 0;
    virtual int     random_int(int low, int high);
    virtual double  random_double();
    virtual double  random_double(double low, double high);
    virtual int     random_byte(char *out, unsigned int out_len, SL_Crypto_Rand::TYPE type, const char *special_char);
    virtual int     random_byte(char *out, unsigned int out_len, int low, int high);

protected:
    virtual char    random_mt_char();                          // 生成标准字符
    virtual char    random_mt_char(const char* special_char);  // 按特殊字符(specialchar)生成 增强型字符

    // Randomizes a buffer
    virtual void    fill_buffer_mt(char *buffer, unsigned int bytes);

    // Randomizes a buffer by type
    virtual void    fill_buffer_mt(char *buffer, unsigned int bytes, int type);
    virtual void    fill_buffer_mt(char *buffer, unsigned int bytes, const char *special_char);
};

#endif

