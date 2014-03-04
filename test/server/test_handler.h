#ifndef TEST_HANDLER_H
#define TEST_HANDLER_H

#include "SL_Config.h"
#include "SL_ObjectPool.h"
#include "SL_Sync_Mutex.h"
#include "SL_Socket_SendControl_Handler.h"
#include "SL_Socket_TcpServer_Handler.h"
#include "SL_Socket_Message_Handler.h"
#include "SL_Socket_Iocp_Message_Handler.h"
#include "SL_Socket_SendControl_HandlerManager.h"
#include "SL_Logger.h"
#include "SL_Seda_Stage.h"
#include "SL_Seda_Event.h"
#include "SL_Rpc_Message.h"

#ifdef SOCKETLITE_OS_WINDOWS
class TestHandler : public SL_Socket_SendControl_Handler<SL_Socket_Iocp_Message_Handler, SL_ByteBuffer, SL_Sync_SpinMutex, SL_Crypto_SymmetricCipher, SL_Crypto_SymmetricCipher>
#else
class TestHandler : public SL_Socket_SendControl_Handler<SL_Socket_Message_Handler, SL_ByteBuffer, SL_Sync_SpinMutex, SL_Crypto_SymmetricCipher, SL_Crypto_SymmetricCipher>
#endif
{
public:
    TestHandler();
    virtual ~TestHandler();

    int do_open();
    int do_message(const char *msg, int len);
    int do_close();

public:
    int     handler_id_;

};

#endif

