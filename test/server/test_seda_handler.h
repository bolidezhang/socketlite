#ifndef TEST_SEDA_HANDLER_H
#define TEST_SEDA_HANDLER_H

#include "SL_Logger.h"
#include "SL_Seda_StageHandler.h"
#include "test_handler.h"

class TestStageHandler : public SL_Seda_StageHandler
{
public:
    TestStageHandler();
    virtual ~TestStageHandler();

    int handle_open();
    int handle_event(const SL_Seda_Event *event);
    int handle_close();

private:
    uint64  stat_recv_count_;
    uint64  stat_total_recv_count_;
    uint64  stat_recv_bytes_;
    uint64  stat_total_recv_bytes_;
};

#endif

