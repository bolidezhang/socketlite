#ifndef SOCKETLITE_SOCKET_TCP_SERVER_H
#define SOCKETLITE_SOCKET_TCP_SERVER_H

#include "SL_Config.h"
#include "SL_Socket_INET_Addr.h"
#include "SL_Socket_Source.h"
#include "SL_Socket_Runner.h"
#include "SL_Thread_Group.h"
#include "SL_ObjectPool.h"
#include "SL_Socket_TcpServer_Handler.h"

template <typename TClientHandler, typename TObjectPool, typename TServerHandler = SL_Socket_TcpServer_Handler>
class SL_Socket_TcpServer : public SL_Socket_Source
{
public:
    SL_Socket_TcpServer()
        : accept_runner_(NULL)
        , object_pool_(NULL)
    {
        set_config();
    }

    virtual ~SL_Socket_TcpServer()
    { 
        close();
    }

    int open(ushort local_port, int backlog = 1024, const char *local_name = NULL, bool is_ipv6 = false)
    {
        close();

        int ret = local_addr_.set(local_name, local_port, is_ipv6);
        if (ret < 0)
        {
            return -1;
        }
        int address_family = is_ipv6 ? AF_INET6:AF_INET;
        SL_SOCKET fd = SL_Socket_CommonAPI::socket_open(address_family, SOCK_STREAM, IPPROTO_TCP);
        if (SL_INVALID_SOCKET == fd)
        {
            return -2;
        }
        if (server_handler_.handle_open(fd, this, socket_runner_) < 0)
        {
            ret = -3;
            goto EXCEPTION_EXIT_PROC;
        }

        SL_Socket_CommonAPI::socket_set_reuseaddr(fd, 1);
        ret = SL_Socket_CommonAPI::socket_bind(fd, local_addr_.get_addr(), local_addr_.get_addr_size());
        if (ret < 0)
        {
            ret = -4;
            goto EXCEPTION_EXIT_PROC;
        }
        ret = SL_Socket_CommonAPI::socket_listen(fd, backlog);
        if (ret < 0)
        {
            ret = -5;
            goto EXCEPTION_EXIT_PROC;
        }
        
#ifdef SOCKETLITE_OS_WINDOWS
        //目前在windows下，只支持在独立的Accept线程接收新连接
        if (accept_thread_num_ < 1)
        {
            accept_thread_num_ = 1;
        }
        SL_Socket_CommonAPI::socket_set_block(fd, true);
        if (accept_thread_group_.start(accept_proc, this, accept_thread_num_, accept_thread_num_) < 0)
        {
            ret = -6;
            goto EXCEPTION_EXIT_PROC;
        }
#else
        if (NULL != accept_runner_)
        {
            if (accept_runner_->add_handle(&server_handler_, SL_Socket_Handler::READ_EVENT_MASK) < 0)
            {
                ret = -7;
                goto EXCEPTION_EXIT_PROC;
            }
        }
        else
        {
            if (accept_thread_num_ > 0)
            {
                SL_Socket_CommonAPI::socket_set_block(fd, true);
                if (accept_thread_group_.start(accept_proc, this, accept_thread_num_, accept_thread_num_) < 0)
                {
                    ret = -8;
                    goto EXCEPTION_EXIT_PROC;
                }
            }
            else
            {
                if (socket_runner_->add_handle(&server_handler_, SL_Socket_Handler::READ_EVENT_MASK) < 0)
                {
                    ret = -9;
                    goto EXCEPTION_EXIT_PROC;
                }
            }
        }
#endif
        socket_handler_ = &server_handler_;
        return 0;

EXCEPTION_EXIT_PROC:
        server_handler_.set_socket(SL_INVALID_SOCKET);
        SL_Socket_CommonAPI::socket_close(fd);
        return ret;
    }

    int close()
    {
        if (NULL != socket_handler_)
        {
            if (accept_thread_num_ > 0)
            {
                accept_thread_group_.stop();
            }
            SL_Socket_CommonAPI::socket_close(server_handler_.get_socket());
            server_handler_.set_socket(SL_INVALID_SOCKET);
            server_handler_.handle_close();
            socket_handler_ = NULL;
        }
        return 0;
    }

    int set_config(ushort accept_thread_num = 1,
                   bool   is_add_runner     = true,
                   uint   maxconnect_num    = 100000,
                   uint   recvbuffer_size   = 4096,
                   uint   msgbuffer_size    = 4096,
                   uint8  msglen_bytes      = 2,
                   uint8  msg_byteorder     = 0)
    {
        accept_thread_num_  = accept_thread_num;
        is_add_runner_      = is_add_runner;
        maxconnect_num_     = maxconnect_num;
        recvbuffer_size_    = recvbuffer_size;
        msgbuffer_size_     = msgbuffer_size;
        msglen_bytes_       = msglen_bytes;
        msg_byteorder_      = msg_byteorder;

        switch (msglen_bytes)
        {
            case 1:
                {
                    get_msglen_proc_  = SL_Socket_Source::get_msglen_int8;
                    set_msglen_proc_  = SL_Socket_Source::set_msglen_int8;
                }
                break;
            case 2:
                if (msg_byteorder)
                {
                    get_msglen_proc_ = SL_Socket_Source::get_msglen_int16_network;
                    set_msglen_proc_ = SL_Socket_Source::set_msglen_int16_network;
                }
                else
                {
                    get_msglen_proc_ = SL_Socket_Source::get_msglen_int16_host;
                    set_msglen_proc_ = SL_Socket_Source::set_msglen_int16_host;
                }
                break;
            case 4:
                if (msg_byteorder)
                {
                    get_msglen_proc_ = SL_Socket_Source::get_msglen_int32_network;
                    set_msglen_proc_ = SL_Socket_Source::set_msglen_int32_network;
                }
                else
                {
                    get_msglen_proc_ = SL_Socket_Source::get_msglen_int32_host;
                    set_msglen_proc_ = SL_Socket_Source::set_msglen_int32_host;
                }
                break;
            default:
                break;
        }

        return 0;
    }
    
    inline int set_interface(TObjectPool *object_pool, SL_Socket_Runner *socket_runner, SL_Socket_Runner *accept_runner = NULL)
    {
        object_pool_    = object_pool;
        socket_runner_  = socket_runner;
        accept_runner_  = accept_runner;
        return 0;
    }

    inline SL_Socket_TcpServer_Handler* get_socket_handler()
    {
        return &server_handler_;
    }

    inline SL_Socket_INET_Addr* get_local_addr()
    {
        return &local_addr_;
    }

    inline SL_Socket_Handler* alloc_handler()
    {
        return object_pool_->alloc_object();
    }

    inline void free_handler(SL_Socket_Handler *socket_handler)
    {
        object_pool_->free_object((TClientHandler *)socket_handler);
        return;
    }

private:
#ifdef SOCKETLITE_OS_WINDOWS
    static unsigned int WINAPI accept_proc(void *arg)
#else
    static void* accept_proc(void *arg)
#endif
    {
        SL_Socket_TcpServer *tcpserver = (SL_Socket_TcpServer *)arg;
        SL_Socket_INET_Addr sl_addr(tcpserver->get_local_addr()->is_ipv6());
        sockaddr *addr  = sl_addr.get_addr();     
        int addrlen     = sl_addr.get_addr_size();

        SL_SOCKET client_socket;
        SL_SOCKET listen_fd = tcpserver->server_handler_.get_socket();

        while (tcpserver->accept_thread_group_.get_running())
        {
            client_socket = SL_Socket_CommonAPI::socket_accept(listen_fd, addr, &addrlen, SL_SOCK_NONBLOCK);
            if (SL_INVALID_SOCKET != client_socket)
            {
                if (tcpserver->get_socket_handler()->handle_accept(client_socket, sl_addr) < 0)
                {
                    SL_Socket_CommonAPI::socket_close(client_socket);
                }
            }
            else
            {//关闭TcpServer::socket_,进而促使accept线程退出
                break;
            }
        }
        return 0;
    }

protected:
    SL_Socket_INET_Addr local_addr_;

    uint                maxconnect_num_;
    ushort              accept_thread_num_;
    SL_Thread_Group     accept_thread_group_;
    SL_Socket_Runner    *accept_runner_;            //接收新连接的运行器
    TObjectPool         *object_pool_;
    TServerHandler      server_handler_;
};

#endif

