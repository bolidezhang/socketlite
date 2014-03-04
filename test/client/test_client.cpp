#include "test_message.h"
#include "test_client.h"

#ifdef SOCKETLITE_OS_WINDOWS
static unsigned int WINAPI send_thread_proc(void *arg)
#else
static void* send_thread_proc(void *arg)
#endif
{
    MsgTestEchoReq  req;
    SL_ByteBuffer   buf(128);

    TestClientApp *app = (TestClientApp *)arg;
    std::list<TestClientApp::TEST_CONNECT* >::iterator iter;
    std::list<TestClientApp::TEST_CONNECT* >::iterator iter_end = app->connect_list_.end();
    app->stat_start_timestamp_ = SL_Socket_CommonAPI::util_timestamp_ms();

    iter = app->connect_list_.begin();
    for (; iter!=iter_end; ++iter)
    {
        for (uint i=0; i<app->send_count_; ++i)
        {
            req.head_.sequence = i;
            buf.reset();
            req.serialize_ex3(&buf);
            (*iter)->put_data(buf.data(), buf.data_size());
        }
    }

    return 0;
}

TestClientApp::TestClientApp()
{
    server_ip_      = "127.0.0.1";
    server_port_    = 2000;
    connect_count_  = 1;
    send_count_     = 50000000;
}

TestClientApp::~TestClientApp()
{
}

int TestClientApp::init()
{
    SL_Socket_CommonAPI::socket_init(2, 2);

    test_obj_pool_.init(10000, 1000, 10);
    handler_mgr_.open(1, 65536*4, 6553, 0, 0, 1, 0, false, true);
    test_runner_.set_buffer_size(65536*4, 65536*4);
    test_runner_.open(SL_Socket_Handler::ALL_EVENT_MASK, 10000, 10, 1);
    stage_.open(1, 5000000, sizeof(SL_Seda_TcpReadMessageEvent), 1, false);

    for (uint i=0; i<connect_count_; ++i)
    {
        TEST_CONNECT *connct = new TEST_CONNECT();
        connct->set_interface(&test_obj_pool_, &test_runner_);
        connct->set_config(0, false, true, 65536*4, 65536*4, 2, 1);
        connct->set_autoconnect(false);

        if (connct->open(server_ip_.c_str(), server_port_) >= 0)
        {
            connect_list_.push_back(connct);
        }
        else
        {
            delete connct;
        }
    }

    return 0;
}

int TestClientApp::run()
{
    SL_Thread thread;
    thread.start(send_thread_proc, this);

#ifdef SOCKETLITE_OS_WINDOWS
    //test_runner_.thread_wait();
    while (1)
    {
        test_runner_.event_loop(10);
    }
#else
    while (1)
    {
        test_runner_.event_loop(10);
    }
#endif

    return 0;
}

#define EXE_VERSION "3.2.23.20131227"
int main(int argc, char* argv[])
{
    printf("test_client version: %s\n", EXE_VERSION);

    if (argc < 5)
    {
        printf("please use the format: server_ip port connect_count send_count (e.g.: 127.0.0.1 2000 1 50000000)\n");
        return 0;
    }
    TestClientApp::instance()->server_ip_       = argv[1];
    TestClientApp::instance()->server_port_     = abs(atoi(argv[2]));
    TestClientApp::instance()->connect_count_   = abs(atoi(argv[3]));
    TestClientApp::instance()->send_count_      = abs(atoi(argv[4]));
    TestClientApp::instance()->total_send_count_ = TestClientApp::instance()->connect_count_  * TestClientApp::instance()->send_count_;

    TestClientApp::instance()->init();
	return TestClientApp::instance()->run();
}

