#ifndef TEST_CLIENT_H
#define TEST_CLIENT_H

#include <list>
#include "SL_Config.h"
#include "SL_Application.h"
#include "SL_ObjectPool.h"
#include "SL_Sync_Mutex.h"
#include "SL_Sync_Guard.h"
#include "SL_Socket_Select_Runner.h"
#include "SL_Socket_Iocp_Runner.h"
#include "SL_Socket_Epoll_Runner.h"
#include "SL_Socket_SendControl_HandlerManager.h"
#include "SL_Socket_TcpClient.h"
#include "SL_Socket_TcpClient_AutoConnect.h"
#include "SL_Seda_Stage.h"
#include "SL_Seda_Event.h"
#include "SL_Rpc_Message.h"
#include "SL_Logger.h"

#include "test_handler.h"
#include "test_seda_handler.h"

class TestClientApp : public SL_Application<TestClientApp>
{
public:
    TestClientApp();
    virtual ~TestClientApp();

    int init();
    int run();

    SL_ObjectPool_SimpleEx<TestHandler, SL_Sync_ThreadMutex> test_obj_pool_;
#ifdef SOCKETLITE_OS_WINDOWS
    //SL_Socket_Iocp_Runner<SL_Sync_SpinMutex>    test_runner_;
    SL_Socket_Select_Runner<SL_Sync_SpinMutex>  test_runner_;
#else
    SL_Socket_Epoll_Runner<SL_Sync_SpinMutex>   test_runner_;
#endif
    SL_Socket_SendControl_HandlerManager        handler_mgr_;

    //seda
    SL_Seda_Stage<TestStageHandler> stage_;

    std::string server_ip_;
    ushort      server_port_;
    uint        connect_count_;
    uint        send_count_;
    uint64      stat_start_timestamp_;
    uint64      total_send_count_;

    typedef SL_Socket_TcpClient<TestHandler, SL_ObjectPool_SimpleEx<TestHandler, SL_Sync_ThreadMutex> > TEST_CONNECT;
    std::list<TEST_CONNECT* > connect_list_;

    SL_Socket_TcpClient_AutoConnect auto_connect_;
};

#endif

