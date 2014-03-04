#include "SL_Socket_SendControl_HandlerManager.h"

#include "test_message.h"
#include "test_handler.h"
#include "test_disruptor_handler.h"
#include "test_server.h"

//Stat统计定时器
const ushort TIMER_STAT = 0;

TestDisruptorHandler::TestDisruptorHandler()
    : stat_recv_count_(0)
    , stat_total_recv_count_(0)
    , stat_recv_bytes_(0)
    , stat_total_recv_bytes_(0)
{
}

TestDisruptorHandler::~TestDisruptorHandler()
{
}

int TestDisruptorHandler::handle_open()
{
    SL_Seda_StageThreadLRUSlotInfo slot[1];
    slot[TIMER_STAT].capacity = 10;
    slot[TIMER_STAT].interval_ms = 1000;
    enable_lru_timers(slot, 1);
    set_lru_timer(TIMER_STAT, NULL);
    return 0;
}

int TestDisruptorHandler::handle_event(const SL_Disruptor_Event *event)
{
    switch (event->get_type())
    {
        case SL_Seda_EventType::THREAD_IDLE:
            break;
        case SL_Seda_EventType::TIMER_EXPIRE:
            {
                SL_Seda_TimerExpireEvent *expire_event = (SL_Seda_TimerExpireEvent *)event;
                switch (expire_event->slot)
                {
                    case TIMER_STAT:
                        {
                            if ( (stat_recv_count_ > 0) || (stat_recv_bytes_ > 0) )
                            {
                                stat_total_recv_count_ += stat_recv_count_;
                                stat_total_recv_bytes_ += stat_recv_bytes_;

                                printf("1 second result: recv_count/s:%d recv_bytes/s:%lld recv_count:%lld recv_bytes:%lld\n", \
                                    stat_recv_count_, stat_recv_bytes_, stat_total_recv_count_, stat_total_recv_bytes_);

                                stat_recv_count_ = 0;
                                stat_recv_bytes_ = 0;
                            }
                            set_lru_timer(TIMER_STAT, NULL);
                        }
                        break;
                    default:
                        break;
                }
            }
            break;
        case SL_Seda_EventType::TCP_READ_MESSAGE:
            {
                SL_Seda_TcpReadMessageEvent *message_event = (SL_Seda_TcpReadMessageEvent *)event;
                SL_Rpc_MessageParser::parse_head_ex3<SL_Rpc_MessageHeadEx3>(message_event->message->data(), message_event->message->data_size());
                SL_Rpc_MessageHeadEx3 *req_head = (SL_Rpc_MessageHeadEx3 *)message_event->message->data();

                ++stat_recv_count_;
                stat_recv_bytes_ += message_event->message->data_size();

                //逻辑处理
                switch (req_head->type)
                {
                    case test::MessageType::MT_ECHO_REQ:
                        {
                            MsgTestEchoRes res;
                            res.head_.sequence = req_head->sequence;
                            message_event->message->reset();
                            res.serialize_ex3(message_event->message);
                            TestHandler *test_handler = (TestHandler *)message_event->socket_handler;
                            test_handler->put_data(*message_event->message);
                        }
                        break;
                    default:
                        break;
                }
                delete message_event->message;
            }
            break;
        default:
            break;
    }
    return 0;
}

int TestDisruptorHandler::handle_close()
{
    return 0;
}

