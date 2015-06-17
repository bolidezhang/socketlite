#ifndef SOCKETLITE_CIRCULAR_BYTEBUFFER_H
#define SOCKETLITE_CIRCULAR_BYTEBUFFER_H
#include "SL_Malloc.h"
#include "SL_Utility_Memory.h"

class SL_CircularBuffer
{
private:
    inline SL_CircularBuffer()
    {
    }

public:
    inline SL_CircularBuffer(uint buffer_size)
        : buffer_size_(buffer_size)
        , data_begin_index_(0)
        , data_end_index_(0)
        , data_size_(0)
    {
        buffer_ = (char *)sl_malloc(buffer_size);
    }

    inline ~SL_CircularBuffer()
    {
        if (NULL == buffer_)
        {
            sl_free(buffer_);
        }
    }

    inline uint buffer_size() const
    {
        return buffer_size_;
    }
    
    inline uint data_size() const
    {
        return data_size_;
    }

    inline void clear()
    {
        data_begin_index_   = 0;
        data_end_index_     = 0;
        data_size_          = 0;
    }

    inline uint write(const char *data, uint size)
    {
        uint capacity    = buffer_size_;
        uint write_bytes = min(size, capacity - size);
        if (capacity - data_end_index_ >= write_bytes)
        {
            sl_memcpy(buffer_ + data_end_index_, data, write_bytes);
            data_end_index_ += write_bytes;
            if (data_end_index_ == capacity)
            {
                data_end_index_ = 0;
            }
        }
        else
        {
            uint size_1 = capacity - data_end_index_;
            sl_memcpy(buffer_ + data_end_index_, data, size_1);
            uint size_2 = write_bytes - size_1;
            sl_memcpy(buffer_, data + size_1, size_2);
            data_end_index_ = size_2;
        }
        data_size_ += write_bytes;
        return write_bytes;
    }

    inline uint read(char *data, uint size)
    {
        uint capacity    = buffer_size_;
        uint read_bytes  = min(size, data_size_);
        if (capacity - data_begin_index_ >= read_bytes)
        {
            sl_memcpy(data, buffer_ + data_begin_index_, read_bytes);
            data_begin_index_ += read_bytes;
            if (data_begin_index_ == capacity)
            {
                data_begin_index_ = 0;
            }
        }
        else
        {
            uint size_1 = capacity - data_begin_index_;
            sl_memcpy(data, buffer_ + data_begin_index_, size_1);
            uint size_2 = read_bytes - size_1;
            sl_memcpy(data + size_1, buffer_, size_2);
            data_begin_index_ = size_2;
        }
        data_size_ -= read_bytes;
        return read_bytes;
    }

private:
    char    *buffer_;
    uint    buffer_size_;
    uint    data_begin_index_;
    uint    data_end_index_;
    uint    data_size_;
};

#endif

