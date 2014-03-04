#include "SL_Socket_CommonAPI.h"
#include "SL_Socket_SendControl_HandlerManager.h"

#include "test_message.h"
#include "test_client.h"
#include "test_handler.h"

TestHandler::TestHandler()
    : handler_id_(0)
{
}

TestHandler::~TestHandler()
{
}

int TestHandler::do_open()
{
    SL_Socket_CommonAPI::socket_set_tcpnodelay(socket_, 1);
    TestClientApp::instance()->handler_mgr_.add_handler(this, this);
    printf("open socket:%d connect size:%d\n", socket_, TestClientApp::instance()->handler_mgr_.handler_size());

    return 0;
}

int TestHandler::do_message(const char *msg, int len)
{
    static uint64 times = 0;
    static uint64 stat_start_timestamp_us = 0;
    static uint64 stat_end_timestamp_us = 0;
    static uint   stat_times = 0;

    ++times;

    //int i = times & 16777215;   //16777216-1
    int i = times % 10000000;
    //int i = times % 2;

    if (i < 2)
    {
        if (i == 1)
        {
            stat_start_timestamp_us = SL_Socket_CommonAPI::util_timestamp_us();
        }
        else
        {//i == 0
            ++stat_times;
            stat_end_timestamp_us = SL_Socket_CommonAPI::util_timestamp_us();
            printf("stat: times:%d tack_time(us):%lld\n",stat_times, (stat_end_timestamp_us - stat_start_timestamp_us));
        }
    }

    SL_Seda_TcpReadMessageEvent event;
    event.socket_handler = this;
    event.message = new SL_ByteBuffer(len);
    event.message->write(msg, len);
    TestClientApp::instance()->stage_.push_event(&event);

    return 0;
}

int TestHandler::do_close()
{
    printf("close socket:%d last_errno:%d status:%d next_status:%d\n", socket_, last_errno_, (int)current_status_, (int)next_status_);
    return 0;
}

