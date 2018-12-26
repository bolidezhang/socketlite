//1)SL_Seda_Stage2解决的问题:
//  因SL_Seda_Stage直接将event推至每个线程的本地队列,
//  若线程处理比较费时, 即使此时其他线程空闲,也不能从本线程的队列取出event进行处理
//2)SL_Seda_Stage2的实现
//  多线程间共享一个全局队列,使每个线程都有event处理,提高整体的处理效率

#ifndef SOCKETLITE_SEDA_STAGE2_H
#define SOCKETLITE_SEDA_STAGE2_H
#include <vector>
#include "SL_DataType.h"
#include "SL_Socket_CommonAPI.h"
#include "SL_Thread.h"
#include "SL_Sync_Mutex.h"
#include "SL_Sync_Condition.h"
#include "SL_Seda_EventQueue.h"
#include "SL_Seda_StageThread2.h"

template <typename TSedaStageHandler>
class SL_Seda_Stage2 : public SL_Seda_IStage
{
public:
    SL_Seda_Stage2()
        : timedwait_interval_us_(1000)
        , timedwait_signal_(true)
        , type_(0)
        , batch_size_(2)
        , is_priority_(false)
    {
    }

    virtual ~SL_Seda_Stage2()
    {
        close();
    }

    int open(uint thread_number,
             uint queue_max_size,
             uint event_len = 64,
             uint timedwait_interval_us = 1000,
             bool timedwait_signal = true,
             int  type = 0,
             uint batch_size = 2,
             bool is_priority = false,
             bool is_shared_queue = true)
    {
        if (thread_number < 1)
        {
            thread_number = 1;
        }
        if (batch_size < 1)
        {
            batch_size = 1;
        }

        timedwait_interval_us_  = timedwait_interval_us;
        timedwait_signal_       = timedwait_signal;
        type_                   = type;
        batch_size_             = batch_size;
        is_priority_            = is_priority;

        low_priority_queue_.init(queue_max_size, event_len);
        if (is_priority)
        {
            high_priority_queue_.init(queue_max_size, event_len);
        }

        stage_threads_.reserve(thread_number);
        StageThread *stage_thread;
        for (uint i=0; i<thread_number; ++i)
        {
            stage_thread = new StageThread(this, i, batch_size, event_len);
            stage_thread->start();
            stage_threads_.push_back(stage_thread);
        }
        return 0;
    }

    int close()
    {
        StageThread *stage_thread;
        typename std::vector<StageThread * >::iterator iter = stage_threads_.begin();
        typename std::vector<StageThread * >::iterator iter_end = stage_threads_.end();
        for (; iter != iter_end; ++iter)
        {
            stage_thread = *iter;
            stage_thread->stop();
            stage_thread->join();
            delete stage_thread;
        }
        stage_threads_.clear();
        return 0;
    }

    int join()
    {
        typename std::vector<StageThread * >::iterator iter = stage_threads_.begin();
        typename std::vector<StageThread * >::iterator iter_end = stage_threads_.end();
        for (; iter != iter_end; ++iter)
        {
            (*iter)->join();
        }
        return 0;
    }

    inline int push_event(const SL_Seda_Event *event, int thread_index = -1, int priority = SL_Seda_Priority::UNKNOWN_PRIOITY)
    {
        int ret= 0;
        if (is_priority_ && (SL_Seda_Priority::HIGH_PRIORITY == priority))
        {
            high_priority_mutex_.lock();
            ret = high_priority_queue_.push(event);
            high_priority_mutex_.unlock();
        }
        else
        {
            low_priority_mutex_.lock();
            ret = low_priority_queue_.push(event);
            low_priority_mutex_.unlock();
        }
        if (timedwait_signal_ && (1 == ret))
        {
            timedwait_condition_.signal();
        }
        return ret;
    }

    inline int pop_event(void *push_queue)
    {
        SL_Seda_EventQueue *push_queue_tmp = (SL_Seda_EventQueue *)push_queue;
        if (is_priority_)
        {
            high_priority_mutex_.lock();
            high_priority_queue_.pop(push_queue_tmp);
            high_priority_mutex_.unlock();
        }
        low_priority_mutex_.lock();
        low_priority_queue_.pop(push_queue_tmp);
        low_priority_mutex_.unlock();

        if (push_queue_tmp->empty())
        {
#ifdef SOCKETLITE_OS_WINDOWS
            timedwait_condition_.timed_wait(NULL, timedwait_interval_us_);
#else
            timedwait_mutex_.lock();
            timedwait_condition_.timed_wait(&timedwait_mutex_, timedwait_interval_us_);
            timedwait_mutex_.unlock();
#endif
        }
        return 0;
    }

    inline int get_type() const 
    {
        return type_;
    }

    inline int get_event_count() const
    {
        return high_priority_queue_.size() + low_priority_queue_.size();
    }

protected:
    typedef SL_Seda_StageThread2<TSedaStageHandler> StageThread;
    std::vector<StageThread * > stage_threads_;

    SL_Seda_EventQueue          high_priority_queue_;
    SL_Sync_SpinMutex           high_priority_mutex_;
    SL_Seda_EventQueue          low_priority_queue_;
    SL_Sync_SpinMutex           low_priority_mutex_;

    SL_Sync_ThreadMutex         timedwait_mutex_;
    SL_Sync_Condition           timedwait_condition_;

    uint    timedwait_interval_us_;
    bool    timedwait_signal_;

    //区分多个实例标识
    int     type_;

    //批处理大小
    uint    batch_size_;

    //是否启用优先级
    bool    is_priority_;
};

#endif

