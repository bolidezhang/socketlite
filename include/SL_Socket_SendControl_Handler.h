#ifndef SOCKETLITE_SOCKET_SEND_CONTROL_HANDLER_H
#define SOCKETLITE_SOCKET_SEND_CONTROL_HANDLER_H

#include <deque>
#include "SL_Config.h"
#include "SL_Crypto.h"
#include "SL_ByteBuffer.h"
#include "SL_Sync_Mutex.h"
#include "SL_Sync_Atomic.h"
#include "SL_Socket_CommonAPI.h"
#include "SL_Socket_Handler.h"
#include "SL_Socket_Source.h"
#include "SL_Socket_Runner.h"
#include "SL_Socket_Message_Handler.h"
#include "SL_Socket_SendControl_HandlerManager.h"

class SL_Socket_SendControl_Interface
{
public:
    // These enumerations are used to describe when messages are delivered.
    enum MessagePriority
    {
        // Used by SocketLite to send above-high priority messages.
        SYSTEM_PRIORITY = 0,

        // High priority messages are send before medium priority messages.
        HIGH_PRIORITY,

        // Medium priority messages are send before low priority messages.
        MEDIUM_PRIORITY,

        // Low priority messages are only sent when no other messages are waiting.
        LOW_PRIORITY,

        // 内部使用
        NUMBER_OF_PRIORITIES
    };

    enum WRITE_RETURN_VALUE
    {
        WRITE_RETURN_NOT_INITIAL    = -3,   //没初始化
        WRITE_RETURN_STATUS_INVALID = -2,   //表示Socket状态失效
        WRITE_RETURN_SEND_FAIL      = -1,   //表示发送失败(Socket已失效)
        WRITE_RETURN_NOT_DATA       = 0,    //表示没有数据可发送 
        WRITE_RETURN_SEND_SUCCESS   = 1,    //表示发送成功
        WRITE_RETURN_SEND_PART      = 2     //表示发送出部分数据(包含0长度)
    };

    SL_Socket_SendControl_Interface()
        : last_update_timestamp_us_(0)
        , last_fail_timestamp_us_(0)
        , handler_manager_(NULL)
        , send_thread_index_(-1)
        , crypto_type_(SL_CRYPTO_TYPE_NOCRYPT)
    {
    }

    virtual ~SL_Socket_SendControl_Interface() 
    {
    }

private:
    virtual void init_control()
    {
    }

    virtual void clear_control()
    {
    }

    virtual WRITE_RETURN_VALUE write_data(SL_IOVEC *iov, int iovcnt, uint64 current_timestamp)
    {
        return WRITE_RETURN_NOT_DATA;
    }

protected:
    uint64  last_update_timestamp_us_;      //最后更新连接时间戳(收到/发送成功数据后更新)
    uint64  last_fail_timestamp_us_;        //最后发送失败时间戳(发送失败后更新)

    SL_Socket_SendControl_HandlerManager *handler_manager_;
    int     send_thread_index_;
    int8    crypto_type_;                   //加解密类型

    friend class SL_Socket_SendControl_HandlerManager;
};

template <typename TSocketHandler, typename TByteBuffer, typename TSyncMutex, typename TEncryptCipher, typename TDecryptCipher>
class SL_Socket_SendControl_Handler : public TSocketHandler, public SL_Socket_SendControl_Interface
{
public:
    SL_Socket_SendControl_Handler()
    {
        queue_active_   = &queue1_;
        queue_standby_  = &queue2_;
    }

    virtual ~SL_Socket_SendControl_Handler()
    {
    }

    inline void clear_data()
    {
        //mutex_.lock();
        //queue1_.clear();
        //queue2_.clear();
        //mutex_.unlock();
    }

    inline size_t get_active_queue_size() const
    {
        return queue_active_->size();
    }

    inline size_t get_standby_queue_size() const
    {
        return queue_standby_->size();
    }

    inline size_t get_sendqueue_size() const
    {
        return queue1_.size() + queue2_.size();
    }

    inline int close_wait()
    {
        mutex_.lock();
        if (SL_Socket_Handler::STATUS_OPEN == TSocketHandler::current_status_)
        {
            TSocketHandler::last_errno_     = SL_Socket_Handler::ERROR_CLOSE_ACTIVE;
            TSocketHandler::current_status_ = SL_Socket_Handler::STATUS_CLOSE_WAIT;
            TSocketHandler::next_status_    = SL_Socket_Handler::STATUS_CLOSE_WAIT;
        }
        mutex_.unlock();
        return 0;
    }

    inline int init_crypt(SL_CRYPTO_TYPE crypto_type, const char *key, const char *iv, int mode = SL_CRYPTO_MODE_ECB, int key_len = 16, int block_size = 16)
    {
        if (crypto_type & SL_CRYPTO_TYPE_ENCRYPT)
        {
            crypto_encrypt_.init((const unsigned char *)key, (const unsigned char *)iv, mode, key_len, block_size);
        }
        if (crypto_type & SL_CRYPTO_TYPE_DECRYPT)
        {
            crypto_decrypt_.init((const unsigned char *)key, (const unsigned char *)iv, mode, key_len, block_size);
        }
        crypto_type_ = crypto_type;
        return 0;
    }

    inline int get_crypto_type()
    {
        return crypto_type_;
    }

    inline int set_crypto_type(SL_CRYPTO_TYPE crypto_type)
    {
        crypto_type_ = crypto_type;
        return 0;
    }

    virtual int do_open()
    {
        return 0;
    }

    virtual int do_close()
    {
        return 0;
    }

    virtual int do_connect()
    {
        return 0;
    }

    //收到一个完整消息时，通知事件
    virtual int do_message(const char *data, int len)
    {
        return 0;
    }

    //数据可发送时，通知事件
    virtual int do_write()
    {
        return 0;
    }

    inline int handle_open(SL_SOCKET fd, SL_Socket_Source *socket_source, SL_Socket_Runner *socket_runner)
    {
        TSocketHandler::handle_open(fd, socket_source, socket_runner);
        return do_open();
    }

    inline int handle_close()
    {
        if (TSocketHandler::current_status_ != SL_Socket_Handler::STATUS_CLOSE)
        {
            do_close();
            TSocketHandler::current_status_ = TSocketHandler::next_status_;
        }
        return -1;
    }

    inline int handle_connect()
    { 
        return do_connect();
    }

    inline int handle_message(const char *data, int len)
    {
        last_update_timestamp_us_ = SL_Socket_SendControl_HandlerManager::current_timestamp_us_;
        if (crypto_type_ & SL_CRYPTO_TYPE_DECRYPT)
        {
            int msglen_bytes = TSocketHandler::get_socket_source()->get_msglen_bytes();
            TByteBuffer tmsg(len);
            int ret = crypto_decrypt_.decrypt((unsigned char *)(data + msglen_bytes), 
                len-msglen_bytes, 
                (unsigned char *)tmsg.buffer(), 
                len);
            if (ret > 0)
            {                
                return do_message(tmsg.buffer(), ret);
            }
            return -1;
        }
        else
        {
            return do_message(data, len);
        }
    }

    inline int handle_write()
    {
        return do_write();
    }

    //发送数据
    //返回值：1)< 0 表示发送失败 2)=0 表示数据进入发送队列 3)>0 数据已发送成功
    inline int put_data(const char *msg, int len)
    {
        if (SL_Socket_Handler::STATUS_OPEN == TSocketHandler::current_status_)
        {
            if (crypto_type_ & SL_CRYPTO_TYPE_ENCRYPT)
            {
                TByteBuffer tmsg(len + SL_CRYPTO_BLOCK_SIZE_32);
                int msglen_bytes = TSocketHandler::get_socket_source()->get_msglen_bytes();
                mutex_.lock();
                int ret = crypto_encrypt_.encrypt((const unsigned char *)msg, 
                    len, 
                    (unsigned char *)(tmsg.buffer() + msglen_bytes), 
                    tmsg.buffer_size() - msglen_bytes);
                if (ret > 0)
                {                
                    int encrypt_len = ret + msglen_bytes;
                    tmsg.data_end(encrypt_len);
                    TSocketHandler::get_socket_source()->set_msglen_proc_(tmsg.buffer(), encrypt_len);
                    ret = put_data_i(tmsg);
                    mutex_.unlock();
                    return ret;
                }
                else
                {
                    mutex_.unlock();
                    return -1;
                }
            }
            else
            {
                mutex_.lock();
                int ret = put_data_i(msg, len);
                mutex_.unlock();
                return ret;
            }
        }
        return -1;
    }

    //发送数据
    //返回值：1)< 0 发送失败 2)=0 数据进入发送队列 3)>0 数据已发送成功
    inline int put_data(TByteBuffer &msg)
    {
        if (SL_Socket_Handler::STATUS_OPEN == TSocketHandler::current_status_)
        {
            if (crypto_type_ & SL_CRYPTO_TYPE_ENCRYPT)
            {
                TByteBuffer tmsg(msg.data_size() + SL_CRYPTO_BLOCK_SIZE_32);
                int msglen_bytes = TSocketHandler::get_socket_source()->get_msglen_bytes();
                mutex_.lock();
                int ret = crypto_encrypt_.encrypt((const unsigned char *)msg.data(), 
                    msg.data_size(), 
                    (unsigned char *)(tmsg.buffer()+msglen_bytes), 
                    tmsg.buffer_size()-msglen_bytes);
                if (ret > 0)
                {
                    int encrypt_len = ret + msglen_bytes;
                    tmsg.data_end(encrypt_len);
                    TSocketHandler::get_socket_source()->set_msglen_proc_(tmsg.buffer(), encrypt_len);
                    ret = put_data_i(tmsg);
                    mutex_.unlock();
                    return ret;
                }
                else
                {
                    mutex_.unlock();
                    return -1;
                }
            }
            else
            {
                mutex_.lock();
                int ret = put_data_i(msg);
                mutex_.unlock();
                return ret;
            }
        }
        return -1;
    }

private:
    inline void init_control()
    {
        mutex_.lock();
        TSocketHandler::last_errno_     = 0;
        TSocketHandler::current_status_ = SL_Socket_Handler::STATUS_OPEN;
        TSocketHandler::next_status_    = SL_Socket_Handler::STATUS_OPEN;
        last_update_timestamp_us_       = SL_Socket_SendControl_HandlerManager::current_timestamp_us_;
        last_fail_timestamp_us_         = 0;
        crypto_type_                    = SL_CRYPTO_TYPE_NOCRYPT;
        queue1_.clear();
        queue2_.clear();
        mutex_.unlock();
    }

    inline void clear_control()
    {
        //通过交换技巧, 释放handler的发送队列占用的内存
        SendQueue swap_queue1;
        SendQueue swap_queue2;

        mutex_.lock();
        last_update_timestamp_us_       = SL_Socket_SendControl_HandlerManager::current_timestamp_us_;
        last_fail_timestamp_us_         = 0;
        crypto_type_                    = SL_CRYPTO_TYPE_NOCRYPT;
        swap_queue1.swap(queue1_);
        swap_queue2.swap(queue2_);
        mutex_.unlock();

        swap_queue1.clear();
        swap_queue2.clear();
    }

    //发送数据
    //返回值的类型: WRITE_RETURN_VALUE
    WRITE_RETURN_VALUE write_data(SL_IOVEC *iov, int iovcnt, uint64 current_timestamp_us)
    {
        if (queue_active_->empty())
        {
            if (!queue_standby_->empty())
            {
                mutex_.lock();
                std::swap(queue_standby_, queue_active_);
                mutex_.unlock();
            }
            else
            {
                return WRITE_RETURN_NOT_DATA;
            }
        }

        int queue_size = queue_active_->size();
        int node_count = (queue_size < iovcnt) ? queue_size : iovcnt;
        typename SendQueue::iterator iter = queue_active_->begin();
        for (int i = 0; i < node_count; ++i)
        {
            iov[i].iov_len  = (*iter).data_size();
            iov[i].iov_base = (*iter).data();
            ++iter;
        }

        //发送数据
        int error_id = 0;
        int send_bytes = SL_Socket_CommonAPI::socket_sendv(TSocketHandler::socket_, iov, node_count, 0, NULL, error_id);
        if (send_bytes > 0)
        {
            last_update_timestamp_us_ = current_timestamp_us;

            int data_size;
            int i = 1;
            for (; i <= node_count; ++i)
            {
                data_size = queue_active_->front().data_size();
                if (send_bytes >= data_size)
                {
                    queue_active_->pop_front();
                    send_bytes -= data_size;
                    if (send_bytes < 1)
                    {
                        break;
                    }
                }
                else
                {
                    queue_active_->front().data_begin(queue_active_->front().data_begin() + send_bytes);
                    break;
                }
            }

            WRITE_RETURN_VALUE ret;
            if (i == node_count)
            {
                last_fail_timestamp_us_ = 0;
                ret = WRITE_RETURN_SEND_SUCCESS;

                //没有数据需要发送时,发出通知事件
                if (queue_active_->empty() && queue_standby_->empty())
                {
                    do_write();
                }
            }
            else
            {
                last_fail_timestamp_us_ = current_timestamp_us;
                ret = WRITE_RETURN_SEND_PART;
            }

            return ret;
        }
        else
        {
            if ( (SL_EAGAIN == error_id) || (SL_EWOULDBLOCK == error_id) || (SL_IO_PENDING == error_id) || (SL_ENOBUFS == error_id) )
            {//非阻塞模式下正常情况
                last_fail_timestamp_us_ = current_timestamp_us;
            }
            else
            {//非阻塞模式下异常情况
                TSocketHandler::last_errno_     = error_id;
                TSocketHandler::current_status_ = SL_Socket_Handler::STATUS_CLOSE_SEND;
                return WRITE_RETURN_SEND_FAIL;
            }
        }
        return WRITE_RETURN_NOT_DATA;
    }

    inline int put_data_i(const char *msg, int len)
    {
        if (handler_manager_->direct_send_flag_ && queue_standby_->empty() && queue_active_->empty())
        {
            uint64 current_timestamp_us = SL_Socket_SendControl_HandlerManager::current_timestamp_us_;
            if (current_timestamp_us - last_fail_timestamp_us_ >= handler_manager_->send_delaytime_us_)
            {//直接发送数据
                int error_id = 0;
                int send_bytes = SL_Socket_CommonAPI::socket_send(TSocketHandler::socket_, msg, len, 0, NULL, &error_id);
                if (send_bytes > 0)
                {
                    last_update_timestamp_us_ = current_timestamp_us;
                    if (send_bytes == len)
                    {
                        return 1;
                    }

                    TByteBuffer tmsg(len - send_bytes);
                    tmsg.write(msg + send_bytes, len - send_bytes);
                    queue_standby_->push_back(tmsg);
                    handler_manager_->wakeup_thread(send_thread_index_);
                    return 0;
                }
                else
                {
                    if ( (SL_EAGAIN == error_id) || (SL_EWOULDBLOCK == error_id) || (SL_IO_PENDING == error_id) || (SL_ENOBUFS == error_id) )
                    {//非阻塞模式下正常情况
                        last_fail_timestamp_us_ = current_timestamp_us;
                    }
                    else
                    {//非阻塞模式下异常情况
                        TSocketHandler::last_errno_     = error_id;
                        TSocketHandler::current_status_ = SL_Socket_Handler::STATUS_CLOSE_SEND;
                        return -1;
                    }
                }
            }
        }

        TByteBuffer tmsg(len);
        tmsg.write(msg, len);
        queue_standby_->push_back(tmsg);
        handler_manager_->wakeup_thread(send_thread_index_);
        return 0;
    }

    inline int put_data_i(TByteBuffer &tmsg)
    {
        if (handler_manager_->direct_send_flag_ && queue_standby_->empty() && queue_active_->empty())
        {
            uint64 current_timestamp_us = SL_Socket_SendControl_HandlerManager::current_timestamp_us_;
            if (current_timestamp_us - last_fail_timestamp_us_ >= handler_manager_->send_delaytime_us_)
            {//直接发送数据
                char *msg = tmsg.data();
                int  len  = tmsg.data_size();
                int  error_id = 0;
                int  send_bytes = SL_Socket_CommonAPI::socket_send(TSocketHandler::socket_, msg, len, 0, NULL, &error_id);
                if (send_bytes > 0)
                {
                    last_update_timestamp_us_ = current_timestamp_us;
                    if (send_bytes == len)
                    {
                        return 1;
                    }

                    tmsg.data_begin(tmsg.data_begin() + send_bytes);
                }
                else
                {
                    if ( (SL_EAGAIN == error_id) || (SL_EWOULDBLOCK == error_id) || (SL_IO_PENDING == error_id) || (SL_ENOBUFS == error_id) )
                    {//非阻塞模式下正常情况
                        last_fail_timestamp_us_ = current_timestamp_us;
                    }
                    else
                    {//非阻塞模式下异常情况
                        TSocketHandler::last_errno_     = error_id;
                        TSocketHandler::current_status_ = SL_Socket_Handler::STATUS_CLOSE_SEND;
                        return -1;
                    }
                }
            }
        }

        queue_standby_->push_back(tmsg);
        handler_manager_->wakeup_thread(send_thread_index_);
        return 0;
    }

protected:
    typedef std::deque<TByteBuffer> SendQueue;

    SendQueue       *queue_active_;
    SendQueue       *queue_standby_;
    SendQueue       queue1_;
    SendQueue       queue2_;
    TSyncMutex      mutex_;
        
    TEncryptCipher  crypto_encrypt_;    //加密
    TDecryptCipher  crypto_decrypt_;    //解密
};

#endif

