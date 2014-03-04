#ifndef TEST_DISRUPTOR_HANDLER_H
#define TEST_DISRUPTOR_HANDLER_H

#include "SL_Logger.h"
#include "SL_Disruptor_Handler.h"

class TestDisruptorHandler : public SL_Disruptor_Handler
{
public:
    TestDisruptorHandler();
    virtual ~TestDisruptorHandler();

    int handle_open();
    int handle_event(const SL_Disruptor_Event *event);
    int handle_close();

private:
    uint64  stat_recv_count_;
    uint64  stat_total_recv_count_;
    uint64  stat_recv_bytes_;
    uint64  stat_total_recv_bytes_;

};

#endif

