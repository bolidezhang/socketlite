#include <algorithm>
#include "SL_Socket_TcpServer_Handler.h"
#include "SL_Socket_SendControl_Handler.h"
#include "SL_Socket_SendControl_HandlerManager.h"

uint64 SL_Socket_SendControl_HandlerManager::current_timestamp_us_ = 0;
uint64 SL_Socket_SendControl_HandlerManager::current_time_us_ = 0;
uint64 SL_Socket_SendControl_HandlerManager::current_timestamp_ = 0;
uint64 SL_Socket_SendControl_HandlerManager::current_time_ = 0;

SL_Socket_SendControl_HandlerManager::SL_Socket_SendControl_HandlerManager()
    : iovec_(NULL)
    , iovec_count_(1024)
    , thread_number_(1)
    , send_block_size_(65536)
    , next_thread_index_(0)
    , timedwait_signal_(true)
    , direct_send_flag_(true)
{
}

SL_Socket_SendControl_HandlerManager::~SL_Socket_SendControl_HandlerManager()
{
    close();
}

int SL_Socket_SendControl_HandlerManager::open(int  thread_number, 
                                               int  send_block_size,
                                               int  iovec_count, 
                                               int  keepalive_time_ms, 
                                               int  send_delaytime_us, 
                                               int  scan_waittime_us,
                                               int  close_delaytime_ms, 
                                               bool timedwait_signal,
                                               bool direct_send_flag)
{
    close();

    iovec_count_ = iovec_count;
    if (iovec_count_ > SL_MAX_IOVCNT)
    {
        iovec_count_    = SL_MAX_IOVCNT;
    }

    thread_number_      = thread_number;
    send_block_size_    = send_block_size;
    keepalive_time_us_  = keepalive_time_ms * 1000;
    send_delaytime_us_  = send_delaytime_us;
    scan_waittime_us_   = scan_waittime_us;
    close_delaytime_us_ = close_delaytime_ms * 1000;
    timedwait_signal_   = timedwait_signal;
    direct_send_flag_   = direct_send_flag;
    if (thread_number > 0)
    {
        send_threads_.reserve(thread_number);
        iovec_ = NULL;
        SendThread *send_thread;
        for (int i=0; i<thread_number; ++i)
        {
            send_thread                         = new SendThread;
            send_thread->parent                 = this;
            send_thread->index                  = i;
            send_thread->handlers_list_size     = 0;
            send_thread->temp_handlers_size     = 0;
            send_thread->temp_handlers_active   = &(send_thread->temp1_handlers);
            send_thread->temp_handlers_standby  = &(send_thread->temp2_handlers);
            send_thread->temp1_handlers.clear();
            send_thread->temp2_handlers.clear();
            send_thread->timedwait_flag.store(0);
            send_thread->thread.start(send_proc, send_thread);
            send_threads_.push_back(send_thread);
        }
    }
    else
    {
        iovec_ = new SL_IOVEC[iovec_count];
        SendThread *send_thread                 = new SendThread;
        send_thread->parent                     = this;
        send_thread->index                      = 0;
        send_thread->handlers_list_size         = 0;
        send_thread->temp_handlers_size         = 0;
        send_thread->temp_handlers_active       = &(send_thread->temp1_handlers);
        send_thread->temp_handlers_standby      = &(send_thread->temp2_handlers);
        send_thread->temp1_handlers.clear();
        send_thread->temp2_handlers.clear();
        send_thread->timedwait_flag.store(0);
        send_threads_.push_back(send_thread);
    }
    return 0;
}

int SL_Socket_SendControl_HandlerManager::close()
{
    if (NULL != iovec_)
    {
        delete []iovec_;
        iovec_ = NULL;
    }
    SendThread *send_thread;
    std::vector<SendThread * >::iterator iter = send_threads_.begin();
    std::vector<SendThread * >::iterator iter_end = send_threads_.end();
    if (thread_number_ > 0)
    {
        for (; iter != iter_end; ++iter)
        {
            send_thread = *iter;
            send_thread->thread.stop();
            send_thread->thread.join();
            send_thread->handlers_list.clear();
            delete send_thread;
        }
    }
    else
    {
        for (; iter != iter_end; ++iter)
        {
            send_thread = *iter;
            send_thread->handlers_list.clear();
            delete send_thread;
        }
    }
    send_threads_.clear();
    return 0;
}

size_t SL_Socket_SendControl_HandlerManager::handler_size()
{
    size_t handler_size = 0;
    SendThread *send_thread;
    std::vector<SendThread * >::iterator iter = send_threads_.begin();
    std::vector<SendThread * >::iterator iter_end = send_threads_.end();
    for (; iter != iter_end; ++iter)
    {
        send_thread = *iter;
        handler_size += (send_thread->temp_handlers_size + send_thread->handlers_list_size);
    }
    return handler_size;
}

int SL_Socket_SendControl_HandlerManager::add_handler(SL_Socket_Handler *handler, SL_Socket_SendControl_Interface *sendcontrol)
{
    sendcontrol->handler_manager_ = this;
    sendcontrol->init_control();

    ITEM_SENDCONTROL item_sendcontrol(handler, sendcontrol);
    int thread_index = assign_thread();
    SendThread *send_thread = send_threads_[thread_index];
    sendcontrol->send_thread_index_ = thread_index;
    send_thread->mutex.lock();
    ++send_thread->temp_handlers_size;
    send_thread->temp_handlers_standby->push_back(item_sendcontrol);
    send_thread->mutex.unlock();

    if (timedwait_signal_)
    {
        if (send_thread->timedwait_flag.load() > 0)
        {
            send_thread->timedwait_flag.store(0);
            send_thread->timedwait_condition.signal();
        }
    }

    return 0;
}

int SL_Socket_SendControl_HandlerManager::event_loop(int timeout_ms)
{   
    SL_Socket_Handler *handler;
    SL_Socket_SendControl_Interface *sendcontrol;
    SendThread *send_thread = send_threads_[0];
    std::list<ITEM_SENDCONTROL>::iterator iter;
    std::list<ITEM_SENDCONTROL>::iterator iter_end = send_thread->handlers_list.end();

    uint64 current_timestamp_us = SL_Socket_CommonAPI::util_timestamp_us();
    current_time_us_            = SL_Socket_CommonAPI::util_time_us();
    current_timestamp_us_       = current_timestamp_us;
    current_timestamp_          = current_timestamp_us_ / 1000LL;
    current_time_               = current_time_us_ / 1000000LL;
    int write_success_times     = 0;

    //交换active/standby指针(使用std::swap函数)
    if (!send_thread->temp_handlers_standby->empty())
    {
        send_thread->mutex.lock();
        send_thread->handlers_list_size += send_thread->temp_handlers_size;
        send_thread->temp_handlers_size  = 0;
        std::swap(send_thread->temp_handlers_standby, send_thread->temp_handlers_active);
        send_thread->mutex.unlock();

        send_thread->handlers_list.insert(iter_end, send_thread->temp_handlers_active->begin(), send_thread->temp_handlers_active->end());
        send_thread->temp_handlers_active->clear();
    }

    iter = send_thread->handlers_list.begin();
    while (iter != iter_end)
    {
        handler     = (*iter).first;
        sendcontrol = (*iter).second;

        switch (handler->current_status_)
        {
            case SL_Socket_Handler::STATUS_OPEN:
                {
                    ++iter;

                    //发送数据
                    if ( (0 == send_delaytime_us_) || (current_timestamp_us - sendcontrol->last_fail_timestamp_us_ >= send_delaytime_us_) )
                    {
                        if (sendcontrol->write_data(iovec_, iovec_count_, current_timestamp_us) == SL_Socket_SendControl_Interface::WRITE_RETURN_SEND_SUCCESS)
                        {
                            ++write_success_times;
                        }
                    }

                    //防止死连接
                    if ( (keepalive_time_us_ > 0) && (current_timestamp_us - sendcontrol->last_update_timestamp_us_ > keepalive_time_us_) )
                    {
                        handler->last_errno_  = SL_Socket_Handler::ERROR_KEEPALIVE_TIMEOUT;
                        handler->next_status_ = SL_Socket_Handler::STATUS_CLOSE;
                        handler->get_socket_runner()->del_handle(handler);
                        handler->current_status_ = SL_Socket_Handler::STATUS_CLOSE;
                    }
                }
                break;
            case SL_Socket_Handler::STATUS_CLOSE_WAIT:
                {
                    ++iter;

                    //发送数据(因为应用主动关闭连接,但可能有数据需要发送)
                    if (sendcontrol->write_data(iovec_, iovec_count_, current_timestamp_us) == SL_Socket_SendControl_Interface::WRITE_RETURN_SEND_SUCCESS)
                    {
                        ++write_success_times;
                    }

                    //延迟关闭连接
                    if ( (0 == close_delaytime_us_) || (current_timestamp_us - sendcontrol->last_update_timestamp_us_ > close_delaytime_us_) )
                    {
                        handler->next_status_ = SL_Socket_Handler::STATUS_CLOSE;
                        handler->get_socket_runner()->del_handle(handler);
                        handler->current_status_ = SL_Socket_Handler::STATUS_CLOSE;
                    }
                }
                break;
            case SL_Socket_Handler::STATUS_CLOSE_SEND:
                {
                    ++iter;
                    handler->next_status_ = SL_Socket_Handler::STATUS_CLOSE;
                    handler->get_socket_runner()->del_handle(handler);
                    handler->current_status_ = SL_Socket_Handler::STATUS_CLOSE;
                }
                break;
            case SL_Socket_Handler::STATUS_CLOSE:
                {
                    //关闭连接
                    handler->close();

                    //从集合中删除handler
                    sendcontrol->clear_control();
                    --send_thread->handlers_list_size;
                    iter = send_thread->handlers_list.erase(iter);

                    //回收连接对象
                    handler->get_socket_source()->free_handler(handler);
                }
                break;
            default:
                ++iter;
                break;
        }
    }

    return write_success_times;
}

#ifdef SOCKETLITE_OS_WINDOWS
unsigned int WINAPI SL_Socket_SendControl_HandlerManager::send_proc(void *arg)
#else
void* SL_Socket_SendControl_HandlerManager::send_proc(void *arg)
#endif
{
    SL_Socket_SendControl_HandlerManager::SendThread *send_thread = (SL_Socket_SendControl_HandlerManager::SendThread *)arg;
    SL_Socket_SendControl_HandlerManager *handler_manger = (SL_Socket_SendControl_HandlerManager *)send_thread->parent;

    SL_Socket_Handler *handler;
    SL_Socket_SendControl_Interface *sendcontrol;
    std::list<ITEM_SENDCONTROL>::iterator iter;
    std::list<ITEM_SENDCONTROL>::iterator iter_end = send_thread->handlers_list.end();

    int iovec_count             = handler_manger->iovec_count_;
    SL_IOVEC *iovec             = new SL_IOVEC[iovec_count];
    uint  write_success_times   = 0;
    uint send_delaytime_us      = handler_manger->send_delaytime_us_;
    uint keepalive_time_us      = handler_manger->keepalive_time_us_;
    uint close_delaytime_us     = handler_manger->close_delaytime_us_;
    uint scan_waittime_us       = handler_manger->scan_waittime_us_;
    uint64 current_timestamp_us = SL_Socket_CommonAPI::util_timestamp_us();
    
    if ( (send_delaytime_us > 0) && (keepalive_time_us > 0) && (close_delaytime_us > 0) && (0 == send_thread->index) )
    {//默认参数下,针对代码的特定优化
        
        while (send_thread->thread.get_running())
        {
            current_timestamp_us                                        = SL_Socket_CommonAPI::util_timestamp_us();
            SL_Socket_SendControl_HandlerManager::current_time_us_      = SL_Socket_CommonAPI::util_time_us();
            SL_Socket_SendControl_HandlerManager::current_timestamp_us_ = current_timestamp_us;
            SL_Socket_SendControl_HandlerManager::current_timestamp_    = current_timestamp_us / 1000LL;
            SL_Socket_SendControl_HandlerManager::current_time_         = SL_Socket_SendControl_HandlerManager::current_time_us_ / 1000000LL;

            //交换active/standby指针(使用std::swap函数)
            if (!send_thread->temp_handlers_standby->empty())
            {
                send_thread->mutex.lock();
                send_thread->handlers_list_size += send_thread->temp_handlers_size;
                send_thread->temp_handlers_size  = 0;
                std::swap(send_thread->temp_handlers_standby, send_thread->temp_handlers_active);
                send_thread->mutex.unlock();

                send_thread->handlers_list.insert(iter_end, send_thread->temp_handlers_active->begin(), send_thread->temp_handlers_active->end());
                send_thread->temp_handlers_active->clear();
            }

            iter = send_thread->handlers_list.begin();
            while (iter != iter_end)
            {
                handler     = (*iter).first;
                sendcontrol = (*iter).second;

                switch (handler->current_status_)
                {
                    case SL_Socket_Handler::STATUS_OPEN:
                        {
                            ++iter;

                            //发送数据
                            if (current_timestamp_us - sendcontrol->last_fail_timestamp_us_ >= send_delaytime_us)
                            {
                                if (sendcontrol->write_data(iovec, iovec_count, current_timestamp_us) == SL_Socket_SendControl_Interface::WRITE_RETURN_SEND_SUCCESS)
                                {
                                    ++write_success_times;
                                }
                            }

                            //防止死连接
                            if (current_timestamp_us - sendcontrol->last_update_timestamp_us_ > keepalive_time_us)
                            {
                                handler->last_errno_  = SL_Socket_Handler::ERROR_KEEPALIVE_TIMEOUT;
                                handler->next_status_ = SL_Socket_Handler::STATUS_CLOSE;
                                handler->get_socket_runner()->del_handle(handler);
                                handler->current_status_ = SL_Socket_Handler::STATUS_CLOSE;
                            }
                        }
                        break;
                    case SL_Socket_Handler::STATUS_CLOSE_WAIT:
                        {
                            ++iter;

                            //发送数据(因为应用主动关闭连接,但可能有数据需要发送)
                            if (sendcontrol->write_data(iovec, iovec_count, current_timestamp_us) == SL_Socket_SendControl_Interface::WRITE_RETURN_SEND_SUCCESS)
                            {
                                ++write_success_times;
                            }

                            //延迟关闭连接
                            if (current_timestamp_us - sendcontrol->last_update_timestamp_us_ > close_delaytime_us)
                            {
                                handler->next_status_ = SL_Socket_Handler::STATUS_CLOSE;
                                handler->get_socket_runner()->del_handle(handler);
                                handler->current_status_ = SL_Socket_Handler::STATUS_CLOSE;
                            }
                        }
                        break;
                    case SL_Socket_Handler::STATUS_CLOSE_SEND:
                        {
                            ++iter;

                            handler->next_status_ = SL_Socket_Handler::STATUS_CLOSE;
                            handler->get_socket_runner()->del_handle(handler);
                            handler->current_status_ = SL_Socket_Handler::STATUS_CLOSE;
                        }
                        break;
                    case SL_Socket_Handler::STATUS_CLOSE:
                        {
                            //关闭连接
                            handler->close();

                            //从集合中删除handler
                            sendcontrol->clear_control();
                            --send_thread->handlers_list_size;
                            iter = send_thread->handlers_list.erase(iter);

                            //回收连接对象
                            handler->get_socket_source()->free_handler(handler);
                        }
                        break;
                    default:
                        ++iter;
                        break;
                    }
            }

            if (write_success_times > 0)
            {
                write_success_times = 0;
            }
            else
            {
                send_thread->timedwait_flag.store(1);
#ifdef SOCKETLITE_OS_WINDOWS
                send_thread->timedwait_condition.timed_wait(NULL, scan_waittime_us);
#else
                send_thread->timedwait_mutex.lock();
                send_thread->timedwait_condition.timed_wait(&send_thread->timedwait_mutex, scan_waittime_us);
                send_thread->timedwait_mutex.unlock();
#endif
                send_thread->timedwait_flag.store(0);
            }
        }
    }
    else
    {//不是默认参数时

        while (send_thread->thread.get_running())
        {
            if (0 == send_thread->index)
            {//第一个发送线程,更新全局时间
                current_timestamp_us                                        = SL_Socket_CommonAPI::util_timestamp_us();
                SL_Socket_SendControl_HandlerManager::current_time_us_      = SL_Socket_CommonAPI::util_time_us();
                SL_Socket_SendControl_HandlerManager::current_timestamp_us_ = current_timestamp_us;
                SL_Socket_SendControl_HandlerManager::current_timestamp_    = current_timestamp_us / 1000LL;
                SL_Socket_SendControl_HandlerManager::current_time_         = SL_Socket_SendControl_HandlerManager::current_time_us_ / 1000000LL;
            }
            else
            {
                current_timestamp_us = SL_Socket_SendControl_HandlerManager::current_timestamp_us_;
            }

            //交换active/standby指针(使用std::swap函数)
            if (!send_thread->temp_handlers_standby->empty())
            {
                send_thread->mutex.lock();
                send_thread->handlers_list_size += send_thread->temp_handlers_size;
                send_thread->temp_handlers_size  = 0;
                std::swap(send_thread->temp_handlers_standby, send_thread->temp_handlers_active);
                send_thread->mutex.unlock();

                send_thread->handlers_list.insert(iter_end, send_thread->temp_handlers_active->begin(), send_thread->temp_handlers_active->end());
                send_thread->temp_handlers_active->clear();
            }

            iter = send_thread->handlers_list.begin();
            while (iter != iter_end)
            {
                handler     = (*iter).first;
                sendcontrol = (*iter).second;

                switch (handler->current_status_)
                {
                    case SL_Socket_Handler::STATUS_OPEN:
                        {
                            ++iter;

                            //发送数据
                            if ( (0 == send_delaytime_us) || (current_timestamp_us - sendcontrol->last_fail_timestamp_us_ >= send_delaytime_us) )
                            {
                                if (sendcontrol->write_data(iovec, iovec_count, current_timestamp_us) == SL_Socket_SendControl_Interface::WRITE_RETURN_SEND_SUCCESS)
                                {
                                    ++write_success_times;
                                }
                            }

                            //防止死连接
                            if ( (keepalive_time_us > 0) && (current_timestamp_us - sendcontrol->last_update_timestamp_us_ > keepalive_time_us) )
                            {
                                handler->last_errno_  = SL_Socket_Handler::ERROR_KEEPALIVE_TIMEOUT;
                                handler->next_status_ = SL_Socket_Handler::STATUS_CLOSE;
                                handler->get_socket_runner()->del_handle(handler);
                                handler->current_status_ = SL_Socket_Handler::STATUS_CLOSE;
                            }
                        }
                        break;
                    case SL_Socket_Handler::STATUS_CLOSE_WAIT:
                        {
                            ++iter;

                            //发送数据(因为应用主动关闭连接,但可能有数据需要发送)
                            if (sendcontrol->write_data(iovec, iovec_count, current_timestamp_us) == SL_Socket_SendControl_Interface::WRITE_RETURN_SEND_SUCCESS)
                            {
                                ++write_success_times;
                            }

                            //延迟关闭连接
                            if ( (0 == close_delaytime_us) || (current_timestamp_us - sendcontrol->last_update_timestamp_us_ > close_delaytime_us) )
                            {
                                handler->next_status_ = SL_Socket_Handler::STATUS_CLOSE;
                                handler->get_socket_runner()->del_handle(handler);
                                handler->current_status_ = SL_Socket_Handler::STATUS_CLOSE;
                            }
                        }
                        break;
                    case SL_Socket_Handler::STATUS_CLOSE_SEND:
                        {
                            ++iter;

                            handler->next_status_ = SL_Socket_Handler::STATUS_CLOSE;
                            handler->get_socket_runner()->del_handle(handler);
                            handler->current_status_ = SL_Socket_Handler::STATUS_CLOSE;
                        }
                        break;
                    case SL_Socket_Handler::STATUS_CLOSE:
                        {
                            //关闭连接
                            handler->close();

                            //从集合中删除handler
                            sendcontrol->clear_control();
                            --send_thread->handlers_list_size;
                            iter = send_thread->handlers_list.erase(iter);

                            //回收连接对象
                            handler->get_socket_source()->free_handler(handler);
                        }
                        break;
                    default:
                        ++iter;
                        break;
                }
            }

            if (write_success_times > 0)
            {
                write_success_times = 0;
            }
            else
            {
                send_thread->timedwait_flag.store(1);
#ifdef SOCKETLITE_OS_WINDOWS
                send_thread->timedwait_condition.timed_wait(NULL, scan_waittime_us);
#else
                send_thread->timedwait_mutex.lock();
                send_thread->timedwait_condition.timed_wait(&send_thread->timedwait_mutex, scan_waittime_us);
                send_thread->timedwait_mutex.unlock();
#endif
                send_thread->timedwait_flag.store(0);
            }
        }
    }

    delete []iovec;
    send_thread->thread.exit();
    return 0;
}

int SL_Socket_SendControl_HandlerManager::wakeup_thread(int thread_index)
{
    if (timedwait_signal_)
    {
        SendThread *send_thread = send_threads_[thread_index];
        if (send_thread->timedwait_flag.load() > 0)
        {
            send_thread->timedwait_flag.store(0);
            send_thread->timedwait_condition.signal();
            return 1;
        }
    }
    return 0;
}

