#ifndef TEST_MESSAGE_H
#define TEST_MESSAGE_H

#include "SL_Rpc_Message.h"
#include "./gen-cpp/test_types.h"

typedef SL_Rpc_Message<SL_Rpc_MessageHeadEx3, SL_Rpc_NULL_MessageBody, test::MessageType::MT_ECHO_REQ>          MsgTestEchoReq;
typedef SL_Rpc_Message<SL_Rpc_MessageHeadEx3, SL_Rpc_NULL_MessageBody, test::MessageType::MT_ECHO_RES>          MsgTestEchoRes;

typedef SL_Rpc_Message<SL_Rpc_MessageHeadEx3, SL_Rpc_NULL_MessageBody, test::MessageType::MT_HELLO_REQ>         MsgTestHelloReq;
typedef SL_Rpc_Message<SL_Rpc_MessageHeadEx3, SL_Rpc_NULL_MessageBody, test::MessageType::MT_HELLO_RES>         MsgTestHelloRes;
typedef SL_Rpc_Message<SL_Rpc_MessageHeadEx3, SL_Rpc_NULL_MessageBody, test::MessageType::MT_HELLO_NOTIFY>      MsgTestHelloNotify;

typedef SL_Rpc_Message<SL_Rpc_MessageHeadEx3, test::KeepAliveReq, test::MessageType::MT_KEEPALIVE_REQ>          MsgTestKeepaliveReq;
typedef SL_Rpc_Message<SL_Rpc_MessageHeadEx3, test::KeepAliveRes, test::MessageType::MT_KEEPALIVE_RES>          MsgTestKeepaliveRes;
typedef SL_Rpc_Message<SL_Rpc_MessageHeadEx3, SL_Rpc_NULL_MessageBody, test::MessageType::MT_KEEPALIVE_NOTIFY>  MsgTestKeepaliveNotify;

#endif

