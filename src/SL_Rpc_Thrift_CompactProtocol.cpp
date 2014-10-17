#include "SL_Rpc_Thrift_CompactProtocol.h"

const int8_t SL_Rpc_Thrift_CompactProtocol_Internal::TTypeToCType[16] = 
{
    SL_Rpc_Thrift_CompactProtocol_Internal::CT_STOP,            // T_STOP
    0,                                                          // unused
    SL_Rpc_Thrift_CompactProtocol_Internal::CT_BOOLEAN_TRUE,    // T_BOOL
    SL_Rpc_Thrift_CompactProtocol_Internal::CT_BYTE,            // T_BYTE
    SL_Rpc_Thrift_CompactProtocol_Internal::CT_DOUBLE,          // T_DOUBLE
    0,                                                          // unused
    SL_Rpc_Thrift_CompactProtocol_Internal::CT_I16,             // T_I16
    0,                                                          // unused
    SL_Rpc_Thrift_CompactProtocol_Internal::CT_I32,             // T_I32
    0,                                                          // unused
    SL_Rpc_Thrift_CompactProtocol_Internal::CT_I64,             // T_I64
    SL_Rpc_Thrift_CompactProtocol_Internal::CT_BINARY,          // T_STRING
    SL_Rpc_Thrift_CompactProtocol_Internal::CT_STRUCT,          // T_STRUCT
    SL_Rpc_Thrift_CompactProtocol_Internal::CT_MAP,             // T_MAP
    SL_Rpc_Thrift_CompactProtocol_Internal::CT_SET,             // T_SET
    SL_Rpc_Thrift_CompactProtocol_Internal::CT_LIST,            // T_LIST
};

