#include "SL_Socket_CommonAPI.h"
#include "test_server.h"

TestServerApp::TestServerApp()
{
}

TestServerApp::~TestServerApp()
{
}

int TestServerApp::run()
{
#ifndef SOCKETLITE_OS_WINDOWS
    signal(SIGPIPE, SIG_IGN);
    signal(SIGSEGV, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
#endif
    SL_Socket_CommonAPI::socket_init(2, 2);

    test_obj_pool_.init(5000, 5000, 10);
    test_tcpserver_.set_interface(&test_obj_pool_, &test_runner_);
    test_tcpserver_.set_config(1, true, 100000, 16384, 16384, 2, 1);
    handler_mgr_.open(1, 65536, 1024, 60000, 0, 1, 0, false, true);
    stage_.open(1, 5000000, 48, 1, false, 0);

    //disruptor_queue1_.init(10000000, 48);
    //disruptor_queue1_.add_handler(&disruptor_handler_, true, true);
    //disruptor_handler_.open(&disruptor_queue1_, true, 1, false);

    //disruptor_queue2_.init(5000000, 48);
    //disruptor_queue2_.add_handler(&disruptor_handler_, true, false);
    //disruptor_handler_.open(&disruptor_queue2_, true, 1, false);

    test_runner_.set_buffer_size(65536*4, 65536*4);
    test_runner_.open(SL_Socket_Handler::ALL_EVENT_MASK, 10000, 10, 1);
    test_tcpserver_.open(port_, 128);

#ifdef SOCKETLITE_OS_WINDOWS
    test_runner_.thread_wait();
#else
    while (1)
    {
        test_runner_.event_loop(10);
    }
#endif

    stage_.close();
    handler_mgr_.close();
    test_runner_.close();
    test_tcpserver_.close();

    return 0;
}

#define EXE_VERSION "3.2.24.20140103"
int main(int argc, char *argv[])
{
    printf("test_server version: %s\n", EXE_VERSION);

    if (argc < 2)
    {
        printf("please use the format: port to launch server(e.g.: 2000)\n");
        return 0;
    }
    TestServerApp::instance()->port_ = atoi(argv[1]);

    return TestServerApp::instance()->run();
}

