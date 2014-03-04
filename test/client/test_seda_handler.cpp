#include "SL_Socket_SendControl_HandlerManager.h"

#include "test_message.h"
#include "test_handler.h"
#include "test_seda_handler.h"
#include "test_client.h"

//Stat统计定时器
const ushort TIMER_STAT = 0;

TestStageHandler::TestStageHandler()
    : stat_recv_count_(0)
    , stat_total_recv_count_(0)
    , stat_recv_bytes_(0)
    , stat_total_recv_bytes_(0)
{
}

TestStageHandler::~TestStageHandler()
{
}

int TestStageHandler::handle_open()
{
    SL_Seda_StageThreadLRUSlotInfo slot[1];
    slot[TIMER_STAT].capacity = 10;
    slot[TIMER_STAT].interval_ms = 1000;
    //stage_thread_->enable_lru_timers(slot, 1);
    //stage_thread_->set_lru_timer(TIMER_STAT, NULL);
    return 0;
}

int TestStageHandler::handle_event(const SL_Seda_Event *event)
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
                                printf("1 second result: recv_count/s:%lld recv_bytes/s:%lld recv_count:%lld recv_bytes:%lld\n", \
                                    stat_recv_count_, stat_recv_bytes_, stat_total_recv_count_, stat_total_recv_bytes_);

                                stat_recv_count_ = 0;
                                stat_recv_bytes_ = 0;
                            }
                            stage_thread_->set_lru_timer(TIMER_STAT, NULL);
                        }
                        break;
                    default:
                        break;
                }
            }
            break;
        case SL_Seda_EventType::TCP_READ_MESSAGE:
            {
                ++stat_recv_count_;
                ++stat_total_recv_count_;
                stat_recv_bytes_        += sizeof(SL_Rpc_MessageHeadEx3);
                stat_total_recv_bytes_  += sizeof(SL_Rpc_MessageHeadEx3);
                if (TestClientApp::instance()->total_send_count_ == stat_total_recv_count_)
                {
                    uint64 recv_count_s;
                    uint64 recv_bytes_s;
                    uint64 end_timestamp = SL_Socket_CommonAPI::util_timestamp_ms();
                    uint64 tack_time_ms  = end_timestamp - TestClientApp::instance()->stat_start_timestamp_;
                    if (tack_time_ms > 0)
                    {
                        recv_count_s = stat_total_recv_count_ * 1000 / tack_time_ms;
                        recv_bytes_s = stat_total_recv_bytes_ * 1000 / tack_time_ms;
                    }
                    else
                    {
                        recv_count_s = stat_total_recv_count_ * 1000;
                        recv_bytes_s = stat_total_recv_bytes_ * 1000;
                    }

                    printf("end tack_time(ms):%lld recv_count/s:%lld recv_bytes/s:%lld recv_count:%lld recv_bytes:%lld\n", \
                        tack_time_ms, recv_count_s, recv_bytes_s, stat_total_recv_count_, stat_total_recv_bytes_);
                }

                SL_Seda_TcpReadMessageEvent *message_event = (SL_Seda_TcpReadMessageEvent *)event;
                SL_Rpc_MessageParser::parse_head_ex3<SL_Rpc_MessageHeadEx3>(message_event->message->data(), message_event->message->data_size());
                SL_Rpc_MessageHeadEx3 *msg_head = (SL_Rpc_MessageHeadEx3 *)message_event->message->data();

                //逻辑处理代码
                switch (msg_head->type)
                {
                    case test::MessageType::MT_ECHO_RES:
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

int TestStageHandler::handle_close()
{
    return 0;
}

