#ifndef SOCKETLITE_QUEUE_H
#define SOCKETLITE_QUEUE_H

#include <deque>
#include "SL_DataType.h"
#include "SL_Sync_Guard.h"

template <typename T, typename TSyncMutex>
class SL_Queue
{
public:
    inline SL_Queue()
        : capacity_size_(100000)
    {
    }

    inline ~SL_Queue()
    {
        clear();
    }

    inline int init(ulong capacity = 100000)
    {
        if (capacity < 1)
        {
            capacity_size_ = 1;
        }
        else
        {
            capacity_size_ = capacity;
        }
        clear();
        return 0;
    }

    inline void clear()
    {
        mutex_.lock();
        queue_.clear();
        mutex_.unlock();
    }

    inline long push_front_i(const T &node)
    {
        queue_.push_front(node);
        return queue_.size();
    }

    inline long push_front_i(const T node[], int node_count, int &push_node_count)
    {
        for (int i=0; i<node_count; ++i)
        {
            queue_.push_front(node[i]);
        }
        push_node_count = node_count;
        return queue_.size();
    }

    inline long push_back_i(const T &node)
    {
        queue_.push_back(node);
        return queue_.size();
    }

    inline long push_back_i(const T node[], int node_count, int &push_node_count)
    {
        for (int i = 0; i < node_count; ++i)
        {
            queue_.push_back(node[i]);
        }
        push_node_count = node_count;
        return queue_.size();
    }

    inline long pop_front_i(T &node)
    {
        if (!queue_.empty())
        {
            node = queue_.front();
            queue_.pop_front();
            return queue_.size();
        }
        return -1;
    }

    inline long pop_front_i(T node[], int node_count, int &pop_node_count)
    {
        int temp_size = queue_.size();
        pop_node_count = ((node_count < temp_size) ? node_count : temp_size);
        if (pop_node_count <= 0)
        {
            return -1;
        }
        for (int i = 0; i < pop_node_count; ++i)
        {
            node[i] = queue_.front();
            queue_.pop_front();
        }
        return queue_.size();
    }

    inline long pop_back_i(T &node)
    {
        if (!queue_.empty())
        {
            node = queue_.back();
            queue_.pop_back();
            return queue_.size();
        }
        return -1;
    }

    inline long pop_back_i(T node[], int node_count, int &pop_node_count)
    {
        int temp_size = queue_.size();
        pop_node_count = ((node_count < temp_size) ? node_count : temp_size);
        if (pop_node_count <= 0)
        {
            return -1;
        }
        for (int i = 0; i < pop_node_count; ++i)
        {
            node[i] = queue_.back();
            queue_.pop_back();
        }
        return queue_.size();
    }

    inline long push_front(const T &node)
    {
        mutex_.lock();
        long ret = push_front_i(node);
        mutex_.unlock();
        return ret;
    }

    inline long push_front(const T node[], int node_count, int &push_node_count)
    {
        mutex_.lock();
        long ret = push_front_i(node, node_count, push_node_count);
        mutex_.unlock();
        return ret;
    }

    inline long push_back(const T &node)
    {
        mutex_.lock();
        long ret = push_back_i(node);
        mutex_.unlock();
        return ret;
    }

    inline long push_back(const T node[], int node_count, int &push_node_count)
    {
        mutex_.lock();
        long ret = push_back_i(node, node_count, push_node_count);
        mutex_.unlock();
        return ret;
    }

    inline long pop_front(T &node)
    {
        mutex_.lock();
        long ret = pop_front_i(node);
        mutex_.unlock();
        return ret;
    }

    inline long pop_front(T node[], int node_count, int &pop_node_count)
    {
        mutex_.lock();
        long ret = pop_front_i(node, node_count, pop_node_count);
        mutex_.unlock();
        return ret;
    }

    inline long pop_back(T &node)
    {
        mutex_.lock();
        long ret = pop_back_i(node);
        mutex_.unlock();
        return ret;
    }

    inline long pop_back(T node[], int node_count, int &pop_node_count)
    {
        mutex_.lock();
        long ret = pop_back_i(node, node_count, pop_node_count);
        mutex_.unlock();
        return ret;
    }

    inline long push(const T &node)
    {
        mutex_.lock();
        long ret = push_back_i(node);
        mutex_.unlock();
        return ret;
    }

    inline long pop(const T &node)
    {
        mutex_.lock();
        long ret = pop_back_i(node);
        mutex_.unlock();
        return ret;
    }

    inline long size() const
    {
        return queue_.size();
    }

    inline bool empty() const
    {
        return queue_.empty();
    }

    inline T& front()
    {
        return queue_.front();
    }

    inline T& back()
    {
        return queue_.back();
    }

    inline TSyncMutex& mutex()
    {
        return mutex_;
    }

private:
    ulong           capacity_size_;
    std::deque<T>   queue_;
    TSyncMutex      mutex_;
};

#endif

