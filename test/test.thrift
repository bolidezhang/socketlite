namespace cpp test

enum MessageType
{
    MT_BEGIN_NUMBER = 10,                       //开始序号
    
    MT_ECHO_REQ = 10,
    MT_ECHO_RES,
    
    MT_HELLO_REQ,
    MT_HELLO_RES,
    MT_HELLO_NOTIFY,
    
    MT_KEEPALIVE_REQ,
    MT_KEEPALIVE_RES,
    MT_KEEPALIVE_NOTIFY,
            
    MT_END_NUMBER,                              //结束序号
}

//消息头SL_Rpc_MessageHeadEx3.attach_id1存service_type_id
struct KeepAliveReq
{
    1:i32       service_id,                     //服务id (若<0: 表示用户发来消息)
    2:i16       service_type_id,                //服务类型id 关联enum ServiceType
    3:i32       current_load_capacity,          //当前负载
    4:i32       req_id,                         //请求id
    5:i64       current_timestamp_req,          //当前时间戳(请求端)
}

struct KeepAliveRes
{
    1:i16       error_code,
    2:string    error_text,
    3:i32       req_id,                         //请求id
    4:i64       current_timestamp_req,          //当前时间戳(请求端)
    5:i64       current_time_res,               //当前时间(回复端) 用于同步时间
}
