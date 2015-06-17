#ifndef SOCKETLITE_SEDA_EVENT_QUEUE_H
#define SOCKETLITE_SEDA_EVENT_QUEUE_H

#include "SL_Config.h"
#include "SL_Seda_Event.h"
#include "SL_Seda_Interface.h"
#include "SL_Utility_Memory.h"

class SL_Seda_EventQueue : public SL_Seda_IEventQueue
{
public:
    inline SL_Seda_EventQueue()
        : pool_begin_(NULL)
        , pool_end_(NULL)
        , write_index_(NULL)
        , read_index_(NULL)
        , capacity_(0)
        , queue_size_(0)
        , event_len_(0)
    {
    }

    virtual ~SL_Seda_EventQueue()
    {
        if (NULL != pool_begin_)
        {
            sl_free(pool_begin_);
        }
    }

    inline int init(uint capacity, uint event_len = 64)
    {
        clear();

        uint pool_size = capacity * event_len;
        pool_begin_ = (char *)sl_malloc(pool_size);
        if (NULL != pool_begin_)
        {
            pool_end_       = pool_begin_ + pool_size;
            write_index_    = pool_begin_;
            read_index_     = pool_begin_;
            capacity_       = capacity;
            queue_size_     = 0;
            event_len_      = event_len;
            return 1;
        }
        return -1;
    }

    inline void clear()
    {
        if (NULL != pool_begin_)
        {
            sl_free(pool_begin_);
            pool_begin_ = NULL;
            pool_end_   = NULL;
        }
        write_index_    = NULL;
        read_index_     = NULL;
        capacity_       = 0;
        queue_size_     = 0;
        event_len_      = 0;
    }

    inline int push(const SL_Seda_Event *event)
    {
        if (queue_size_ < capacity_)
        {
            sl_memcpy(write_index_, event->get_event_buffer(), event->get_event_len());
            if (write_index_ + event_len_ < pool_end_)
            {
                write_index_ += event_len_;
            }
            else
            {
                write_index_ = pool_begin_;
            }
            return ++queue_size_;
        }
        return -1;
    }

    inline SL_Seda_Event* pop()
    {
        if (queue_size_ > 0)
        {
            --queue_size_;

            SL_Seda_Event *event = (SL_Seda_Event *)read_index_;
            if (read_index_ + event_len_ < pool_end_)
            {
                read_index_ += event_len_;
            }
            else
            {
                read_index_ = pool_begin_;
            }
            return event;
        }
        return NULL;
    }

    inline int capacity() const
    {
        return capacity_;
    }

    inline int size() const
    {
        return queue_size_;
    }

    inline bool empty() const
    {
        return 0 == queue_size_;
    }

private:
    char  *pool_begin_;         //事件对象池开始位
    char  *pool_end_;           //事件对象池结束位
    char  *write_index_;        //写位置
    char  *read_index_;         //读位置

    int   capacity_;            //队列容量
    int   queue_size_;          //队列大小
    uint  event_len_;           //事件对象大小
};

#endif

