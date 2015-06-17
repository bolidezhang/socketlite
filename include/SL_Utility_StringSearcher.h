#ifndef SOCKETLITE_UTILITY_STRING_SEARCHER_H
#define SOCKETLITE_UTILITY_STRING_SEARCHER_H

#include <assert.h>
#include <vector>
#include "SL_DataType.h"
#include "SL_Utility_Memory.h"
#include "SL_Config.h"

class SL_Utility_StringSearcher
{
public:
    SL_Utility_StringSearcher()
    {
    }

    ~SL_Utility_StringSearcher()
    {
    }

    static inline char* search(const char *str, uint str_len, const char *search, uint search_len)
    {
#define COMPARE_INT64                                   \
        do {                                            \
            str64 = (int64 *)(ret + i);                 \
            if (*str64 == *search64)                    \
            {                                           \
                goto MATCHING_PROC_INT64;               \
            }                                           \
            ++i;                                        \
        } while (0)

#define COMPARE_INT64_RET                               \
        do {                                            \
            str64 = (int64 *)(ret + i);                 \
            if (*str64 == *search64)                    \
            {                                           \
                return (ret + i);                       \
            }                                           \
            ++i;                                        \
        } while (0)

#define COMPARE_INT32                                   \
        do {                                            \
            str32 = (int32 *)(ret + i);                 \
            if (*str32 == *search32)                    \
            {                                           \
                goto MATCHING_PROC_INT32;               \
            }                                           \
            ++i;                                        \
        } while (0)

#define COMPARE_INT32_RET                               \
        do {                                            \
            str32 = (int32 *)(ret + i);                 \
            if (*str32 == *search32)                    \
            {                                           \
                return (ret + i);                       \
            }                                           \
            ++i;                                        \
        } while (0)

#define COMPARE_INT16                                   \
        do {                                            \
            str16 = (int16 *)(ret + i);                 \
            if (*str16 == *search16)                    \
            {                                           \
                goto MATCHING_PROC_INT16;               \
            }                                           \
            ++i;                                        \
        } while (0)

#define COMPARE_INT16_RET                               \
        do {                                            \
            str16 = (int16 *)(ret + i);                 \
            if (*str16 == *search16)                    \
            {                                           \
                return (ret + i);                       \
            }                                           \
            ++i;                                        \
        } while (0)

#define COMPARE_CHAR                                    \
        do {                                            \
            if (ret[i] == search[0])                    \
            {                                           \
                return (ret + i);                       \
            }                                           \
            ++i;                                        \
        } while (0)

        if ( (0 == str_len) || (0 == search_len) || (str_len < search_len) )
        {
            return NULL;
        }
        char *ret = (char *)str;
        uint i = 0;

        if (search_len > sizeof(int64))
        {
            int64 *search64 = (int64 *)search;
            for (;;)
            {
                int64 *str64    = (int64 *)(ret + i);
                uint trip_count = (str_len - i) >> 4;
                for (; trip_count > 0; --trip_count)
                {
                    COMPARE_INT64;
                    COMPARE_INT64;
                    COMPARE_INT64;
                    COMPARE_INT64;
                    COMPARE_INT64;
                    COMPARE_INT64;
                    COMPARE_INT64;
                    COMPARE_INT64;
                    COMPARE_INT64;
                    COMPARE_INT64;
                    COMPARE_INT64;
                    COMPARE_INT64;
                    COMPARE_INT64;
                    COMPARE_INT64;
                    COMPARE_INT64;
                    COMPARE_INT64;
                }
                switch (str_len - i)
                {
                    case 15:
                        COMPARE_INT64;
                    case 14:
                        COMPARE_INT64;
                    case 13:
                        COMPARE_INT64;
                    case 12:
                        COMPARE_INT64;
                    case 11:
                        COMPARE_INT64;
                    case 10:
                        COMPARE_INT64;
                    case 9:
                        COMPARE_INT64;
                    case 8:
                        COMPARE_INT64;
                }
                return NULL;

MATCHING_PROC_INT64:
                i += sizeof(int64);
                if (i >= str_len)
                {
                    return NULL;
                }

                uint k = i++;
                uint j = sizeof(int64);
                while (ret[k] == search[j])
                {
                    if (++j == search_len)
                    {
                        if (k < str_len)
                            return (ret + k - j + 1);
                        else
                            return NULL;
                    }
                    ++k;
                }
            }
        }
        else if (search_len == sizeof(int64))
        {
            int64 *search64 = (int64 *)search;
            int64 *str64    = (int64 *)ret;
            uint trip_count = (str_len - i) >> 4;
            for (; trip_count > 0; --trip_count)
            {
                COMPARE_INT64_RET;
                COMPARE_INT64_RET;
                COMPARE_INT64_RET;
                COMPARE_INT64_RET;
                COMPARE_INT64_RET;
                COMPARE_INT64_RET;
                COMPARE_INT64_RET;
                COMPARE_INT64_RET;
                COMPARE_INT64_RET;
                COMPARE_INT64_RET;
                COMPARE_INT64_RET;
                COMPARE_INT64_RET;
                COMPARE_INT64_RET;
                COMPARE_INT64_RET;
                COMPARE_INT64_RET;
                COMPARE_INT64_RET;
            }
            switch (str_len - i)
            {
                case 15:
                    COMPARE_INT64_RET;
                case 14:
                    COMPARE_INT64_RET;
                case 13:
                    COMPARE_INT64_RET;
                case 12:
                    COMPARE_INT64_RET;
                case 11:
                    COMPARE_INT64_RET;
                case 10:
                    COMPARE_INT64_RET;
                case 9:
                    COMPARE_INT64_RET;
                case 8:
                    COMPARE_INT64_RET;
            }
            return NULL;
        }
        else if (search_len > sizeof(int32))
        {
            int32 *search32 = (int32 *)search;
            for (;;)
            {
                int32 *str32    = (int32 *)(ret + i);
                uint trip_count = (str_len - i) >> 4;
                for (; trip_count > 0; --trip_count)
                {
                    COMPARE_INT32;
                    COMPARE_INT32;
                    COMPARE_INT32;
                    COMPARE_INT32;
                    COMPARE_INT32;
                    COMPARE_INT32;
                    COMPARE_INT32;
                    COMPARE_INT32;
                    COMPARE_INT32;
                    COMPARE_INT32;
                    COMPARE_INT32;
                    COMPARE_INT32;
                    COMPARE_INT32;
                    COMPARE_INT32;
                    COMPARE_INT32;
                    COMPARE_INT32;
                }
                switch (str_len - i)
                {
                    case 15:
                        COMPARE_INT32;
                    case 14:
                        COMPARE_INT32;
                    case 13:
                        COMPARE_INT32;
                    case 12:
                        COMPARE_INT32;
                    case 11:
                        COMPARE_INT32;
                    case 10:
                        COMPARE_INT32;
                    case 9:
                        COMPARE_INT32;
                    case 8:
                        COMPARE_INT32;
                    case 7:
                        COMPARE_INT32;
                    case 6:
                        COMPARE_INT32;
                    case 5:
                        COMPARE_INT32;
                    case 4:
                        COMPARE_INT32;
                }
                return NULL;

MATCHING_PROC_INT32:
                i += sizeof(int32);
                if (i >= str_len)
                {
                    return NULL;
                }

                uint k = i++;
                uint j = sizeof(int32);
                while (ret[k] == search[j])
                {
                    if (++j == search_len)
                    {
                        if (k < str_len)
                            return (ret + k - j + 1);
                        else
                            return NULL;
                    }
                    ++k;
                }
            }
        }
        else if (search_len == sizeof(int32))
        {
            int32 *search32 = (int32 *)search;
            int32 *str32    = (int32 *)ret;
            uint trip_count = (str_len - i) >> 4;
            for (; trip_count > 0; --trip_count)
            {
                COMPARE_INT32_RET;
                COMPARE_INT32_RET;
                COMPARE_INT32_RET;
                COMPARE_INT32_RET;
                COMPARE_INT32_RET;
                COMPARE_INT32_RET;
                COMPARE_INT32_RET;
                COMPARE_INT32_RET;
                COMPARE_INT32_RET;
                COMPARE_INT32_RET;
                COMPARE_INT32_RET;
                COMPARE_INT32_RET;
                COMPARE_INT32_RET;
                COMPARE_INT32_RET;
                COMPARE_INT32_RET;
                COMPARE_INT32_RET;
            }
            switch (str_len - i)
            {
                case 15:
                    COMPARE_INT32_RET;
                case 14:
                    COMPARE_INT32_RET;
                case 13:
                    COMPARE_INT32_RET;
                case 12:
                    COMPARE_INT32_RET;
                case 11:
                    COMPARE_INT32_RET;
                case 10:
                    COMPARE_INT32_RET;
                case 9:
                    COMPARE_INT32_RET;
                case 8:
                    COMPARE_INT32_RET;
                case 7:
                    COMPARE_INT32_RET;
                case 6:
                    COMPARE_INT32_RET;
                case 5:
                    COMPARE_INT32_RET;
                case 4:
                    COMPARE_INT32_RET;
            }
            return NULL;
        }
        else if (search_len > sizeof(int16))
        {
            int16 *search16 = (int16 *)search;
            for (;;)
            {
                int16 *str16    = (int16 *)(ret + i);
                uint trip_count = (str_len - i) >> 4;
                for (; trip_count > 0; --trip_count)
                {
                    COMPARE_INT16;
                    COMPARE_INT16;
                    COMPARE_INT16;
                    COMPARE_INT16;
                    COMPARE_INT16;
                    COMPARE_INT16;
                    COMPARE_INT16;
                    COMPARE_INT16;
                    COMPARE_INT16;
                    COMPARE_INT16;
                    COMPARE_INT16;
                    COMPARE_INT16;
                    COMPARE_INT16;
                    COMPARE_INT16;
                    COMPARE_INT16;
                    COMPARE_INT16;
                }
                switch (str_len - i)
                {
                    case 15:
                        COMPARE_INT16;
                    case 14:
                        COMPARE_INT16;
                    case 13:
                        COMPARE_INT16;
                    case 12:
                        COMPARE_INT16;
                    case 11:
                        COMPARE_INT16;
                    case 10:
                        COMPARE_INT16;
                    case 9:
                        COMPARE_INT16;
                    case 8:
                        COMPARE_INT16;
                    case 7:
                        COMPARE_INT16;
                    case 6:
                        COMPARE_INT16;
                    case 5:
                        COMPARE_INT16;
                    case 4:
                        COMPARE_INT16;
                    case 3:
                        COMPARE_INT16;
                    case 2:
                        COMPARE_INT16;
                }
                return NULL;

MATCHING_PROC_INT16:
                i += sizeof(int16);
                if (i >= str_len)
                {
                    return NULL;
                }

                uint k = i++;
                uint j = sizeof(int16);
                while (ret[k] == search[j])
                {
                    if (++j == search_len)
                    {
                        if (k < str_len)
                            return (ret + k - j + 1);
                        else
                            return NULL;
                    }
                    ++k;
                }
            }
        }
        else if (search_len == sizeof(int16))
        {
            int16 *search16 = (int16 *)search;
            int16 *str16    = (int16 *)ret;
            uint trip_count = (str_len - i) >> 4;
            for (; trip_count > 0; --trip_count)
            {
                COMPARE_INT16_RET;
                COMPARE_INT16_RET;
                COMPARE_INT16_RET;
                COMPARE_INT16_RET;
                COMPARE_INT16_RET;
                COMPARE_INT16_RET;
                COMPARE_INT16_RET;
                COMPARE_INT16_RET;
                COMPARE_INT16_RET;
                COMPARE_INT16_RET;
                COMPARE_INT16_RET;
                COMPARE_INT16_RET;
                COMPARE_INT16_RET;
                COMPARE_INT16_RET;
                COMPARE_INT16_RET;
                COMPARE_INT16_RET;
            }
            switch (str_len - i)
            {
                case 15:
                    COMPARE_INT16_RET;
                case 14:
                    COMPARE_INT16_RET;
                case 13:
                    COMPARE_INT16_RET;
                case 12:
                    COMPARE_INT16_RET;
                case 11:
                    COMPARE_INT16_RET;
                case 10:
                    COMPARE_INT16_RET;
                case 9:
                    COMPARE_INT16_RET;
                case 8:
                    COMPARE_INT16_RET;
                case 7:
                    COMPARE_INT16_RET;
                case 6:
                    COMPARE_INT16_RET;
                case 5:
                    COMPARE_INT16_RET;
                case 4:
                    COMPARE_INT16_RET;
                case 3:
                    COMPARE_INT16_RET;
                case 2:
                    COMPARE_INT16_RET;
            }
            return NULL;
        }
        else
        {//search_len == 1
            uint trip_count = (str_len - i) >> 4;
            for (; trip_count > 0; --trip_count)
            {
                COMPARE_CHAR;
                COMPARE_CHAR;
                COMPARE_CHAR;
                COMPARE_CHAR;
                COMPARE_CHAR;
                COMPARE_CHAR;
                COMPARE_CHAR;
                COMPARE_CHAR;
                COMPARE_CHAR;
                COMPARE_CHAR;
                COMPARE_CHAR;
                COMPARE_CHAR;
                COMPARE_CHAR;
                COMPARE_CHAR;
                COMPARE_CHAR;
                COMPARE_CHAR;
            }
            switch (str_len - i)
            {
                case 15:
                    COMPARE_CHAR;
                case 14:
                    COMPARE_CHAR;
                case 13:
                    COMPARE_CHAR;
                case 12:
                    COMPARE_CHAR;
                case 11:
                    COMPARE_CHAR;
                case 10:
                    COMPARE_CHAR;
                case 9:
                    COMPARE_CHAR;
                case 8:
                    COMPARE_CHAR;
                case 7:
                    COMPARE_CHAR;
                case 6:
                    COMPARE_CHAR;
                case 5:
                    COMPARE_CHAR;
                case 4:
                    COMPARE_CHAR;
                case 3:
                    COMPARE_CHAR;
                case 2:
                    COMPARE_CHAR;
                case 1:
                    COMPARE_CHAR;
            }
        }
        return NULL;
    }

#undef COMPARE_INT64
#undef COMPARE_INT64_RET
#undef COMPARE_INT32
#undef COMPARE_INT32_RET
#undef COMPARE_INT16
#undef COMPARE_INT16_RET
#undef COMPARE_CHAR
};

//KMP查找子串算法
class SL_Utility_KMPStringSearcher
{
public:
    SL_Utility_KMPStringSearcher()
    {
    }

    ~SL_Utility_KMPStringSearcher()
    {
    }

    void init(const char *search, uint len)
    {
        next_.reserve(len);
        next_.clear();
        next_.resize(len);

        next_[0] = 0;
        uint j   = 0;
        for (uint i = 1; i < len; ++i)
        {
            while (j > 0 && search[i] != search[j])
            {
                j = next_[j - 1];
            }
            if (search[i] == search[j])
            {
                ++j;
            }
            next_[i] = j;
        }
    }

    char* search(const char *str, uint str_len, const char *search, uint search_len)
    {
        uint j = 0;
        for (uint i = 0; i < str_len; ++i)
        {
            while (j > 0 && str[i] != search[j])
            {
                j = next_[j - 1];
            }
            if (str[i] == search[j])
            {
                ++j;
            }
            if (j == search_len)
            {
                return (char *)(str + i - j + 1);
            }
        }
        return NULL;
    }

private:
    //匹配数组
    std::vector<uint> next_;
};

//Boyer-Moore(BM)查找子串算法
class SL_Utility_BMStringSearcher
{
public:
    SL_Utility_BMStringSearcher()
    {
    }

    ~SL_Utility_BMStringSearcher()
    {
    }

    char* search(const char *str, uint str_len, const char *search, uint search_len);
};

#ifdef SOCKETLITE_OS_WINDOWS
    #define SL_STRSTR(str, str_len, search, search_len) std::strstr(str, search)
    #define sl_strstr(str, str_len, search, search_len) SL_Utility_StringSearcher::search(str, str_len, search, search_len)
#else
    #define SL_STRSTR(str, str_len, search, search_len) std::strstr(str, search)
    #define sl_strstr(str, str_len, search, search_len) SL_Utility_StringSearcher::search(str, str_len, search, search_len)
#endif

#endif

