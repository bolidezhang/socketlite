#include "SL_Socket_CommonAPI.h"
#include "SL_Socket_SendControl_HandlerManager.h"

#include "test_message.h"
#include "test_server.h"
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
    TestServerApp::instance()->handler_mgr_.add_handler(this, this);
    printf("open socket:%d connect size:%d\n", socket_, TestServerApp::instance()->handler_mgr_.handler_size());

    return 0;
}

int TestHandler::do_message(const char *msg, int len)
{
    //if (STATUS_OPEN != current_status_)
    //{
    //    return 0;
    //}

    //SL_Rpc_MessageParser::parse_head_ex3<SL_Rpc_MessageHeadEx3>((char *)msg, len);
    //SL_Rpc_MessageHeadEx3 *msg_head = (SL_Rpc_MessageHeadEx3 *)msg;

    //int ret = -1;
    //SL_Seda_RpcMessageEvent rpc_event;
    //switch (msg_head->type)
    //{
    //    case test::MessageType::MT_KEEPALIVE_REQ:
    //        {
    //            MsgTestKeepaliveReq req;
    //            req.parse((char *)msg, len);

    //            MsgTestKeepaliveRes res;
    //            res.head_.flag = req.head_.flag;
    //            res.head_.reserved = req.head_.reserved;
    //            res.head_.source = req.head_.source;
    //            res.head_.destination = req.head_.destination;
    //            res.head_.sequence = req.head_.sequence;
    //            res.head_.attach_id1 = req.head_.attach_id1;
    //            res.head_.attach_id2 = req.head_.attach_id2;
    //            res.body_.error_code = 200;
    //            res.body_.error_text = "OK";
    //            res.body_.req_id = req.body_.req_id;
    //            res.body_.current_time_res = SL_Socket_CommonAPI::util_time_us();
    //            res.body_.current_timestamp_req = req.body_.current_timestamp_req;

    //            SL_ByteBuffer buf(1024);
    //            res.serialize_ex3(&buf);
    //            this->put_data(buf);
    //        }
    //        break;
    //}
    //return 0;

    static uint64 times = 0;
    static uint64 stat_start_timestamp_us = 0;
    static uint64 stat_end_timestamp_us = 0;
    static uint   stat_times = 0;

    ++times;

    //int i = times & 16777215;   //16777216-1
    int i = times % 10000000;

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
    TestServerApp::instance()->stage_.push_event(&event);
    //TestServerApp::instance()->disruptor_queue1_.push(&event);
    //TestServerApp::instance()->disruptor_queue2_.push(&event);

    return 0;
}

int TestHandler::do_close()
{
    printf("close socket:%d last_errno:%d status:%d next_status:%d\n", socket_, last_errno_, (int)current_status_, (int)next_status_);
    return 0;
}

