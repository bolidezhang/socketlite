////////////////////////////////////////////////////////////////////////////////
//  basic structure of seda event

#ifndef SOCKETLITE_SEDA_EVENT_H
#define SOCKETLITE_SEDA_EVENT_H
#include <stddef.h>
#include "SL_DataType.h"
#include "SL_ByteBuffer.h"
#include "SL_Seda_Timer.h"
#include "SL_Socket_Handler.h"
#include "SL_Socket_INET_Addr.h"

////////////////////////////////////////////////////////////////////////////////
//event types

////////////////////////////////////////////////////////////////////////////////
//basic event structure
//NOTICE: this class can be inherited, 
//      but NEVER define virtual member-functions in the sub-classes

class SL_Seda_Event
{
public:
    inline uint8* get_event_buffer() const
    {
        return ((uint8 *)this);
    }

    inline int get_type() const
    {
        return type_;
    }

    inline int get_event_len() const
    {
        return len_;
    }

    inline int64 get_timestamp() const
    {
        return timestamp_;
    }

    inline void set_timestamp(int64 timestamp)
    {
        timestamp_ = timestamp;
    }

public:
    inline SL_Seda_Event(int type, int len)
        : type_(type)
        , len_(len)
    {
    }

    inline ~SL_Seda_Event()
    {
    }

    int     type_;      //type of SL_Seda_Event
    int     len_;       //len of data, include sizeof(SL_Seda_Event)
    int64   timestamp_; //事件发生时间(建议时间单位:us, 可以自行定义) 用于控制SEDA线程处理太慢,事件在队列等待时间过长时,无需再处理此事件)
};

////////////////////////////////////////////////////////////////////////////////
//template of signal events (events without data)
//  type_value: the number of event type

template <int type_value>
class SL_Seda_SignalEvent : public SL_Seda_Event
{
public:
    inline SL_Seda_SignalEvent(int type = type_value) 
        : SL_Seda_Event(type, sizeof(SL_Seda_Event))
    {
    }

    inline ~SL_Seda_SignalEvent()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////
//template of fixed-size events
//  TEvent: the final class name of the event
//  type_value: the number of event type

template <class TEvent, int type_value>
class SL_Seda_FixedSizeEventBase : public SL_Seda_Event
{
protected:
    inline SL_Seda_FixedSizeEventBase(int type = type_value)
        : SL_Seda_Event(type, sizeof(TEvent))
    {
    }

    inline ~SL_Seda_FixedSizeEventBase()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////
//  base event types
struct SL_Seda_EventType
{
    enum
    {
        QUIT_EVENT = 0,
        THREAD_IDLE = 1,
        TIMER_EXPIRE = 2,

        TCP_LISTEN = 3,
        TCP_SOCKET_OPEN = 4,
        TCP_SOCKET_CLOSE = 5,
        TCP_READ_DATA = 6,
        TCP_READ_MESSAGE = 7,
        TCP_WRITE_DATA = 8,

        UDP_SOCKET_OPEN = 9,
        UDP_SOCKET_CLOSE =10,
        UDP_READ_DATA = 11,
        UDP_WRITE_DATA = 12,

        RPC_MESSAGE = 13,               //rpc(支持thrift和protobuf等消息格式)

        USER_START = 1000,
        USER_START_NUMBER = 1000,
    };
};

typedef SL_Seda_SignalEvent<SL_Seda_EventType::QUIT_EVENT>  SL_Seda_QuitEvent;
typedef SL_Seda_SignalEvent<SL_Seda_EventType::THREAD_IDLE> SL_Seda_ThreadIdleEvent;

struct SL_Seda_TimerExpireEvent : public SL_Seda_Event
{
    SL_Seda_Timer *timer;
    int slot;

    inline SL_Seda_TimerExpireEvent()
        : SL_Seda_Event(SL_Seda_EventType::TIMER_EXPIRE, sizeof(SL_Seda_TimerExpireEvent))
    {
    }

    inline ~SL_Seda_TimerExpireEvent()
    {
    }
};

struct SL_Seda_TcpSocketOpenEvent : SL_Seda_Event
{
    SL_Socket_Handler *socket_handler;

    inline SL_Seda_TcpSocketOpenEvent()
        : SL_Seda_Event(SL_Seda_EventType::TCP_SOCKET_OPEN, sizeof(SL_Seda_TcpSocketOpenEvent))
    {
    }

    inline ~SL_Seda_TcpSocketOpenEvent()
    {
    }
};

struct SL_Seda_TcpSocketCloseEvent : SL_Seda_Event
{
    SL_Socket_Handler *socket_handler;
    int64 attach_id1;
    int64 attach_id2;
    void  *attach_object1;
    void  *attach_object2;

    inline SL_Seda_TcpSocketCloseEvent()
        : SL_Seda_Event(SL_Seda_EventType::TCP_SOCKET_CLOSE, sizeof(SL_Seda_TcpSocketCloseEvent))
    {
    }

    inline ~SL_Seda_TcpSocketCloseEvent()
    {
    }
};

struct SL_Seda_TcpReadDataEvent : public SL_Seda_Event
{
    SL_Socket_Handler *socket_handler;
    SL_ByteBuffer *data;
    int64 attach_id1;
    int64 attach_id2;
    void  *attach_object1;
    void  *attach_object2;

    inline SL_Seda_TcpReadDataEvent()
        : SL_Seda_Event(SL_Seda_EventType::TCP_READ_DATA, sizeof(SL_Seda_TcpReadDataEvent))
    {
    }

    inline ~SL_Seda_TcpReadDataEvent()
    {
    }
};

struct SL_Seda_TcpReadMessageEvent : public SL_Seda_Event
{
    SL_Socket_Handler *socket_handler;
    SL_ByteBuffer *message;
    int64 attach_id1;
    int64 attach_id2;
    void  *attach_object1;
    void  *attach_object2;

    inline SL_Seda_TcpReadMessageEvent()
        : SL_Seda_Event(SL_Seda_EventType::TCP_READ_MESSAGE, sizeof(SL_Seda_TcpReadDataEvent))
    {
    }

    inline ~SL_Seda_TcpReadMessageEvent()
    {
    }
};

struct SL_Seda_TcpWriteDataEvent : public SL_Seda_Event
{
    SL_Socket_Handler *socket_handler;
    int64 attach_id1;
    int64 attach_id2;
    void  *attach_object1;
    void  *attach_object2;

    inline SL_Seda_TcpWriteDataEvent()
        : SL_Seda_Event(SL_Seda_EventType::TCP_WRITE_DATA, sizeof(SL_Seda_TcpWriteDataEvent))
    {
    }

    inline ~SL_Seda_TcpWriteDataEvent()
    {
    }
};

typedef int (* SL_Seda_RpcMessageProc)(int socket_handler_id, SL_Socket_Handler *socket_handler, void *seda_stagehandler, void *rpc_message, SL_Socket_INET_Addr *remote_addr);
struct SL_Seda_RpcMessageEvent : public SL_Seda_Event
{
    int socket_handler_id;
    SL_Socket_Handler *socket_handler;
    SL_Seda_RpcMessageProc rpc_proc;
    void *rpc_message;
    SL_Socket_INET_Addr *remote_addr;

    inline SL_Seda_RpcMessageEvent()
        : SL_Seda_Event(SL_Seda_EventType::RPC_MESSAGE, sizeof(SL_Seda_RpcMessageEvent))
    {
    }

    inline ~SL_Seda_RpcMessageEvent()
    {
    }
};

#endif

