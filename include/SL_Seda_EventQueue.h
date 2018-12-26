#ifndef SOCKETLITE_SEDA_EVENT_QUEUE_H
#define SOCKETLITE_SEDA_EVENT_QUEUE_H

#include "SL_Config.h"
#include "SL_Seda_Event.h"
#include "SL_Seda_Interface.h"
#include "SL_Utility_Memory.h"
#include "SL_Utility_Math.h"

class SL_Seda_EventQueue
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

        uint pool_size  = capacity * event_len;
        pool_begin_     = (char *)sl_malloc(pool_size);
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

    inline int pop(SL_Seda_EventQueue *push_queue)
    {
        uint push_free_size = push_queue->free_size();
        if ((queue_size_ < 1) || (push_free_size < 1))
        {
            return -1;
        }

        //方法1
        uint pop_size = SL_MIN(queue_size_, push_free_size);
        for (uint i = 0; i < pop_size; ++i)
        {
            push_queue->push(pop());
        }
        return pop_size;

        //方法2
        //以下代码会导致程序崩,原因未知
        //uint pop_size       = SL_MIN(queue_size_, push_free_size);
        //uint pop_event_size = pop_size * event_len_;
        //sl_memcpy(push_queue->write_index_, read_index_, pop_event_size);
        //if (read_index_ + pop_event_size < pool_end_)
        //{
        //    read_index_ += pop_event_size;
        //}
        //else
        //{
        //    read_index_ = pool_begin_;
        //}
        //queue_size_ -= pop_size;
        //if (push_queue->write_index_ + pop_event_size < push_queue->pool_end_)
        //{
        //    push_queue->write_index_ += pop_event_size;
        //}
        //else
        //{
        //    push_queue->write_index_ = push_queue->pool_begin_;
        //}
        //push_queue->queue_size_ += pop_size;
        //return pop_size;
    }

    inline uint capacity() const
    {
        return capacity_;
    }

    inline uint size() const
    {
        return queue_size_;
    }
    
    inline uint free_size() const
    {
        return capacity_ - queue_size_;
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

    uint  capacity_;            //队列容量
    uint  queue_size_;          //队列大小
    uint  event_len_;           //事件对象大小
};

#endif

