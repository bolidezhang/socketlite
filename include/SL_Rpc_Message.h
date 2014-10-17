#ifndef SOCKETLITE_RPC_MESSAGE_H
#define SOCKETLITE_RPC_MESSAGE_H

#include <vector>
#include <set>

#include "SL_Config.h"
#include "SL_ByteBuffer.h"
#include "SL_Rpc_Thrift_BinaryProtocol.h"
#include "SL_Rpc_Thrift_CompactProtocol.h"
#include "SL_Rpc_Thrift_BufferTransport.h"
#include "SL_Seda_Event.h"
#include "SL_Socket_CommonAPI.h"

enum SL_RPC_MESSAGE_ENCODE_TYPE
{
    SL_RPC_MESSAGE_ENCODE_THRIFT = 0,                   //facebook thrift-BinaryProtocol
    SL_RPC_MESSAGE_ENCODE_PROTOBUF = 1,                 //google protobuf: varint encode/decode implementation
    SL_RPC_MESSAGE_ENCODE_MESSAGEPACK = 2,              //messagepack.org
    SL_RPC_MESSAGE_ENCODE_THRIFT_COMPACT_PROTOCOL = 3,  //facebook thrift-CompactProtocol: varint encode/decode implementation
    SL_RPC_MESSAGE_ENCODE_THRIFT_DENSE_PROTOCOL = 4,    //facebook thrift-DenseProtocol(experimental)
};

//消息标记的取值: 一个或几个按位组合
enum SL_RPC_MESSAGE_FLAG
{
    SL_RPC_MESSAGE_FLAG_BROADCAST = 1,                  //广播(同等于本地广播)
    SL_RPC_MESSAGE_FLAG_LOCAL_BROADCAST = 1,            //本地广播(多播:多个接收者的广播)
    SL_RPC_MESSAGE_FLAG_GLOBAL_BROADCAST = 2,           //全局广播
    SL_RPC_MESSAGE_FLAG_COMPRESS = 4,                   //压缩
    SL_RPC_MESSAGE_FLAG_CRYPTO = 8,                     //加密
    SL_RPC_MESSAGE_FLAG_NOT_DESTINATIONS = 16,          //消息头中的destination不在目标集合中
};

struct SL_Rpc_MessageHead       //24个字节
{
    uint    length;             //长度
    ushort  flag;               //消息标记
    ushort  type;               //消息类型
    uint    source;             //源(发送者)
    uint    destination;        //目标(接收者: 当flag为广播时有多个接收者,  低16位表示为接收者列表的开始位=消息头大小+消息体大小, 高16位第一个接收者在接收者集合中位置)
    uint    sequence;           //消息序列号
    int     attach_id;          //附加id

    static inline SL_Rpc_MessageHead* parse(const char *data, int len)
    {        
        SL_Rpc_MessageHead *head  = (SL_Rpc_MessageHead *)data;
        head->length        = len;
        head->flag          = ntohs(head->flag);
        head->type          = ntohs(head->type);
        head->source        = ntohl(head->source);
        head->destination   = ntohl(head->destination);
        head->sequence      = ntohl(head->sequence);
        head->attach_id     = ntohl(head->attach_id);
        return head;
    }

    static inline void serialize(SL_Rpc_MessageHead *head)
    {
        head->length        = htonl(head->length);
        head->flag          = htons(head->flag);
        head->type          = htons(head->type);
        head->source        = htonl(head->source);
        head->destination   = htonl(head->destination);
        head->sequence      = htonl(head->sequence);
        head->attach_id     = htonl(head->attach_id);
    }

    static inline void serialize(SL_Rpc_MessageHead *head, int destination_index)
    {
        head->length        = htonl(head->length);
        head->flag          = htons(head->flag);
        head->type          = htons(head->type);
        head->source        = htonl(head->source);
        head->destination   = htonl((destination_index << 16) | head->destination);
        head->sequence      = htonl(head->sequence);
        head->attach_id     = htonl(head->attach_id);
    }

    static inline uint parse_destination(uint dest)
    {        
        return ntohl(dest);
    }

    static inline void parse_destination(SL_Rpc_MessageHead *head, int &destination_index)
    {
        destination_index   = head->destination >> 16;
        head->destination  &= 0xffff;
    }

    static inline uint serialize_destination(uint dest)
    {
        return htonl(dest);
    }
};

struct SL_Rpc_MessageHeadEx     //40个字节
{
    uint    length;             //长度
    ushort  flag;               //消息标记
    ushort  type;               //消息类型
    uint    source;             //源(发送者)
    uint    destination;        //目标(接收者: 当flag为广播时有多个接收者,  低16位表示为接收者列表的开始位=消息头大小+消息体大小, 高16位第一个接收者在接收者集合中位置)
    uint    sequence;           //消息序列号
    int     checksum;           //检验和(消息体的检验和)
    int     attach_id1;         //附加id1
    int     attach_id2;         //附加id2
    int     attach_id3;         //附加id3
    int     attach_id4;         //附加id4

    static inline SL_Rpc_MessageHeadEx* parse(const char *data, int len)
    {
        SL_Rpc_MessageHeadEx *head = (SL_Rpc_MessageHeadEx *)data;
        head->length        = len;
        head->flag          = ntohs(head->flag);
        head->type          = ntohs(head->type);
        head->source        = ntohl(head->source);
        head->destination   = ntohl(head->destination);
        head->sequence      = ntohl(head->sequence);
        head->checksum      = ntohl(head->checksum);
        head->attach_id1    = ntohl(head->attach_id1);
        head->attach_id2    = ntohl(head->attach_id2);
        head->attach_id3    = ntohl(head->attach_id3);
        head->attach_id4    = ntohl(head->attach_id4);
        return head;
    }

    static inline void serialize(SL_Rpc_MessageHeadEx *head)
    {
        head->length        = htonl(head->length);
        head->flag          = htons(head->flag);
        head->type          = htons(head->type);
        head->source        = htonl(head->source);
        head->destination   = htonl(head->destination);
        head->sequence      = htonl(head->sequence);
        head->checksum      = htonl(head->checksum);
        head->attach_id1    = htonl(head->attach_id1);
        head->attach_id2    = htonl(head->attach_id2);
        head->attach_id3    = htonl(head->attach_id3);
        head->attach_id4    = htonl(head->attach_id4);
    }

    static inline void serialize(SL_Rpc_MessageHeadEx *head, int destination_index)
    {
        head->length        = htonl(head->length);
        head->flag          = htons(head->flag);
        head->type          = htons(head->type);
        head->source        = htonl(head->source);
        head->destination   = htonl((destination_index << 16) | head->destination);
        head->sequence      = htonl(head->sequence);
        head->checksum      = htonl(head->checksum);
        head->attach_id1    = htonl(head->attach_id1);
        head->attach_id2    = htonl(head->attach_id2);
        head->attach_id3    = htonl(head->attach_id3);
        head->attach_id4    = htonl(head->attach_id4);
    }

    static inline uint parse_destination(uint dest)
    {        
        return ntohl(dest);
    }

    static inline void parse_destination(SL_Rpc_MessageHeadEx *head, int &destination_index)
    {
        destination_index   = head->destination >> 16;
        head->destination  &= 0xffff;
    }

    static inline uint serialize_destination(uint dest)
    {
        return htonl(dest);
    }
};

struct SL_Rpc_MessageHeadEx2    //64个字节
{
    uint    length;             //长度
    uint    flag;               //消息标记
    uint    type;               //消息类型
    int     checksum;           //检验和(消息体的检验和)
    uint64  source;             //源(发送者)
    uint64  destination;        //目标(接收者: 当flag为广播时有多个接收者,  低32位表示为接收者列表的开始位=消息头大小+消息体大小, 高32位第一个接收者在接收者集合中位置)
    uint64  sequence;           //消息序列号
    int64   attach_id1;         //附加id1
    int64   attach_id2;         //附加id2
    int     attach_id3;         //附加id3
    int     attach_id4;         //附加id4

    static inline SL_Rpc_MessageHeadEx2* parse(const char *data, int len)
    {
        SL_Rpc_MessageHeadEx2 *head = (SL_Rpc_MessageHeadEx2 *)data;
        head->length        = len;
        head->flag          = ntohl(head->flag);
        head->type          = ntohl(head->type);
        head->checksum      = ntohl(head->checksum);
        head->source        = SL_Socket_CommonAPI::util_ntohll(head->source);
        head->destination   = SL_Socket_CommonAPI::util_ntohll(head->destination);
        head->sequence      = SL_Socket_CommonAPI::util_ntohll(head->sequence);
        head->attach_id1    = SL_Socket_CommonAPI::util_ntohll(head->attach_id1);
        head->attach_id2    = SL_Socket_CommonAPI::util_ntohll(head->attach_id2);
        head->attach_id3    = ntohl(head->attach_id3);
        head->attach_id4    = ntohl(head->attach_id4);
        return head;
    }

    static inline void serialize(SL_Rpc_MessageHeadEx2 *head)
    {
        head->length        = htonl(head->length);
        head->flag          = htonl(head->flag);
        head->type          = htonl(head->type);
        head->checksum      = htonl(head->checksum);
        head->source        = SL_Socket_CommonAPI::util_htonll(head->source);
        head->destination   = SL_Socket_CommonAPI::util_htonll(head->destination);
        head->sequence      = SL_Socket_CommonAPI::util_htonll(head->sequence);
        head->attach_id1    = SL_Socket_CommonAPI::util_htonll(head->attach_id1);
        head->attach_id2    = SL_Socket_CommonAPI::util_htonll(head->attach_id2);
        head->attach_id3    = htonl(head->attach_id3);
        head->attach_id4    = htonl(head->attach_id4);
    }

    static inline void serialize(SL_Rpc_MessageHeadEx2 *head, int destination_index)
    {
        head->length        = htonl(head->length);
        head->flag          = htonl(head->flag);
        head->type          = htonl(head->type);
        head->checksum      = htonl(head->checksum);
        head->source        = SL_Socket_CommonAPI::util_htonll(head->source);
        head->destination   = SL_Socket_CommonAPI::util_htonll(((uint64)destination_index << 32) | head->destination);
        head->sequence      = SL_Socket_CommonAPI::util_htonll(head->sequence);
        head->attach_id1    = SL_Socket_CommonAPI::util_htonll(head->attach_id1);
        head->attach_id2    = SL_Socket_CommonAPI::util_htonll(head->attach_id2);
        head->attach_id3    = htonl(head->attach_id3);
        head->attach_id4    = htonl(head->attach_id4);
    }

    static inline uint64 parse_destination(uint64 dest)
    {        
        return SL_Socket_CommonAPI::util_ntohll(dest);
    }

    static inline void parse_destination(SL_Rpc_MessageHeadEx2 *head, int &destination_index)
    {
        destination_index   = head->destination >> 32;
        head->destination  &= 0xffffffff;
    }

    static inline uint64 serialize_destination(uint64 dest)
    {
        return SL_Socket_CommonAPI::util_htonll(dest);
    }
};

struct SL_Rpc_MessageHeadEx3    //40个字节
{
    ushort  length;             //长度
    ushort  flag;               //消息标记
    ushort  type;               //消息类型
    short   reserved;           //保留
    uint64  source;             //源(发送者)
    uint64  destination;        //目标(接收者: 当flag为广播时有多个接收者, 低32位表示为接收者列表的开始位=消息头大小+消息体大小, 高32位第一个接收者在接收者集合中位置)
    uint    sequence;           //消息序列号
    int     attach_id1;         //附加id1
    int64   attach_id2;         //附加id2

    static inline SL_Rpc_MessageHeadEx3* parse(const char *data, int len)
    {
        SL_Rpc_MessageHeadEx3 *head = (SL_Rpc_MessageHeadEx3 *)data;
        head->length        = len;
        head->flag          = ntohs(head->flag);
        head->type          = ntohs(head->type);
        head->reserved      = ntohs(head->reserved);
        head->source        = SL_Socket_CommonAPI::util_ntohll(head->source);
        head->destination   = SL_Socket_CommonAPI::util_ntohll(head->destination);
        head->sequence      = ntohl(head->sequence);
        head->attach_id1    = ntohl(head->attach_id1);
        head->attach_id2    = SL_Socket_CommonAPI::util_ntohll(head->attach_id2);
        return head;
    }

    static inline void serialize(SL_Rpc_MessageHeadEx3 *head)
    {
        head->length        = htons(head->length);
        head->flag          = htons(head->flag);
        head->type          = htons(head->type);
        head->reserved      = htons(head->reserved);
        head->source        = SL_Socket_CommonAPI::util_htonll(head->source);
        head->destination   = SL_Socket_CommonAPI::util_htonll(head->destination);
        head->sequence      = htonl(head->sequence);
        head->attach_id1    = htonl(head->attach_id1);
        head->attach_id2    = SL_Socket_CommonAPI::util_htonll(head->attach_id2);
    }

    static inline void serialize(SL_Rpc_MessageHeadEx3 *head, int destination_index)
    {
        head->length        = htons(head->length);
        head->flag          = htons(head->flag);
        head->type          = htons(head->type);
        head->reserved      = htons(head->reserved);
        head->source        = SL_Socket_CommonAPI::util_htonll(head->source);
        head->destination   = SL_Socket_CommonAPI::util_htonll(((uint64)destination_index << 32) | head->destination);
        head->sequence      = htonl(head->sequence);
        head->attach_id1    = htonl(head->attach_id1);
        head->attach_id2    = SL_Socket_CommonAPI::util_htonll(head->attach_id2);
    }

    static inline uint64 parse_destination(uint64 dest)
    {        
        return SL_Socket_CommonAPI::util_ntohll(dest);
    }

    static inline void parse_destination(SL_Rpc_MessageHeadEx3 *head, int &destination_index)
    {
        destination_index   = head->destination >> 32;
        head->destination  &= 0xffffffff;
    }

    static inline uint64 serialize_destination(uint64 dest)
    {
        return SL_Socket_CommonAPI::util_htonll(dest);
    }
};

class SL_Rpc_MessageParser
{
public:
    SL_Rpc_MessageParser()
    {
    }

    ~SL_Rpc_MessageParser()
    {
    }

    template<typename TMessage> 
    inline static int message_to_event(char *data, 
        int len, 
        int socket_handler_id, 
        SL_Socket_Handler *socket_handler, 
        SL_Seda_RpcMessageProc rpc_proc, 
        SL_Seda_RpcMessageEvent *rpc_event, 
        SL_Socket_INET_Addr *remote_addr)
    {
        TMessage *message = new TMessage;
        if (NULL == message)
        {
            return -1;
        }

        if (message->parse(data, len))
        {
            rpc_event->socket_handler_id = socket_handler_id;
            rpc_event->socket_handler = socket_handler;
            rpc_event->rpc_proc = rpc_proc;
            rpc_event->rpc_message = message;
            rpc_event->remote_addr = remote_addr;
            return 0;
        }
        else
        {
            delete message;
        }
        return -2;
    };

    template<typename THead>
    inline static THead* parse_head(char *data, int len)
    {        
        return THead::parse(data, len);
    }

    template<typename THead>
    inline static int serialize_head(THead *head)
    {
        int len = head->length;
        THead::serialize(head);
        return len;
    }

    template<typename THead, typename TDestinationType, typename TDestinations>
    inline static THead* parse_head(char *data, int len, TDestinations &destinations)
    {
        THead *head = THead::parse(data, len);
        if (head->flag & SL_RPC_MESSAGE_FLAG_LOCAL_BROADCAST)
        {
            bool not_destinations   = head->flag & SL_RPC_MESSAGE_FLAG_NOT_DESTINATIONS;
            int  destination_index  = 0;
            THead::parse_destination(head, destination_index);
            head->length    = head->destination;
            char *dest_pos  = data + head->destination;
            char *data_end  = data + len;
            TDestinationType dest_item;
            if (destination_index > 0)
            {
                int i = 0;
                while (dest_pos < data_end)
                {
                    dest_item = THead::parse_destination( *(TDestinationType *)dest_pos );
                    dest_pos += sizeof(TDestinationType);
                    ++i;
                    if (i == destination_index)
                    {
                        head->destination = dest_item;
                        if ( !not_destinations )
                        {
                            destinations.insert(destinations.end(), dest_item);
                        }
                    }
                    else
                    {
                        destinations.insert(destinations.end(), dest_item);
                    }
                }
            }
            else
            {
                head->destination = 0;
                while (dest_pos < data_end)
                {
                    dest_item = THead::parse_destination( *(TDestinationType *)dest_pos );
                    destinations.insert(destinations.end(), dest_item);
                    dest_pos += sizeof(TDestinationType);
                }
            }
        }
        else
        {
            if (head->destination > 0)
            {
                destinations.insert(destinations.end(), head->destination);
            }
            else if (head->source > 0)
            {
                destinations.insert(destinations.end(), head->source);
            }
        }
        return head;
    }

    template<typename THead, typename TDestinationType, typename TDestinations>
    inline static int serialize_head(THead *head, TDestinations &destinations)
    {
        head->flag &= ~(SL_RPC_MESSAGE_FLAG_LOCAL_BROADCAST | SL_RPC_MESSAGE_FLAG_NOT_DESTINATIONS);
        if (!destinations.empty())
        {
            uint dest_size;
            if (head->destination > 0)
            {
                uint temp_size = destinations.size();
                destinations.insert(destinations.end(), head->destination);
                dest_size = destinations.size();
                if (temp_size < dest_size)
                {
                    head->flag |= SL_RPC_MESSAGE_FLAG_NOT_DESTINATIONS;
                }

                if (dest_size > 1)
                {
                    int destination_index = 0;
                    TDestinationType dest_item;
                    typename TDestinations::iterator iter = destinations.begin();
                    char *dest_pos = (char *)head + head->length;
                    for (uint i = 0; i < dest_size; ++i)
                    {
                        if (*iter == head->destination)
                        {
                            destination_index = i + 1;
                        }
                        dest_item = THead::serialize_destination( *iter );
                        memcpy(dest_pos, (const char *)&dest_item, sizeof(TDestinationType));
                        dest_pos += sizeof(TDestinationType);
                        ++iter;
                    }

                    head->flag |= SL_RPC_MESSAGE_FLAG_LOCAL_BROADCAST;
                    head->destination = head->length;
                    head->length += sizeof(TDestinationType) * dest_size;
                    int len = head->length;
                    THead::serialize(head, destination_index);
                    return len;
                }
            }
            else
            {
                dest_size = destinations.size();
                if (dest_size > 1)
                {
                    TDestinationType dest_item;
                    typename TDestinations::iterator iter = destinations.begin();
                    char *dest_pos = (char *)head + head->length;
                    for (uint i = 0; i < dest_size; ++i)
                    {
                        dest_item = THead::serialize_destination( *iter );
                        memcpy(dest_pos, (const char *)&dest_item, sizeof(TDestinationType));
                        dest_pos += sizeof(TDestinationType);
                        ++iter;
                    }

                    head->flag |= SL_RPC_MESSAGE_FLAG_LOCAL_BROADCAST;
                    head->destination = head->length;
                    head->length += sizeof(TDestinationType) * dest_size;
                    int len = head->length;
                    THead::serialize(head);
                    return len;
                }
                else if (dest_size == 1)
                {
                    head->destination = *destinations.begin();
                }
            }
        }
        else
        {
            if (head->destination <= 0)
            {
                head->destination = head->source;
            }
        }

        int len = head->length;
        THead::serialize(head);
        return len;
    }

    //过时被废弃(用于向后兼容). 新项目请使用parse_head
    template<typename THead>
    inline static THead* parse_head_ex(char *data, int len)
    {        
        return THead::parse(data, len);
    }

    //过时被废弃,向后兼容. 新项目请使用serialize_head
    template<typename THead>
    inline static void serialize_head_ex(THead *head)
    {
        THead::serialize(head);
    }

    //过时被废弃(用于向后兼容). 新项目请使用parse_head
    template<typename THead>
    inline static THead* parse_head_ex2(char *data, int len)
    {        
        return THead::parse(data, len);
    }

    //过时被废弃,向后兼容. 新项目请使用serialize_head
    template<typename THead>
    inline static void serialize_head_ex2(THead *head)
    {
        THead::serialize(head);
    }

    //过时被废弃(用于向后兼容). 新项目请使用parse_head
    template<typename THead>
    inline static THead* parse_head_ex3(char *data, int len)
    {        
        return THead::parse(data, len);
    }

    //过时被废弃(用于向后兼容). 新项目请使用serialize_head
    template<typename THead>
    inline static void serialize_head_ex3(THead *head)
    {
        THead::serialize(head);
    }

    template <typename TBody>
    inline static bool parse_to_body(TBody *body, char *data, int len, SL_RPC_MESSAGE_ENCODE_TYPE encode_type = SL_RPC_MESSAGE_ENCODE_THRIFT)
    {
#ifdef SOCKETLITE_USE_THRIFT
        switch (encode_type)
        {
            case SL_RPC_MESSAGE_ENCODE_THRIFT:
                {
                    SL_Rpc_Thrift_BinaryProtocol<SL_ByteBuffer> thrift_protocol;
                    thrift_protocol.setBuffer(data, len, NULL);

                    try
                    {
                        body->read(&thrift_protocol);
                    }
                    catch (...)
                    {
                        return false;
                    }
                }
                break;
            case SL_RPC_MESSAGE_ENCODE_THRIFT_COMPACT_PROTOCOL:
                {
                    SL_Rpc_Thrift_CompactProtocol<SL_ByteBuffer> thrift_protocol;
                    thrift_protocol.setBuffer(data, len, NULL);

                    try
                    {
                        body->read(&thrift_protocol);
                    }
                    catch (...)
                    {
                        return false;
                    }
                }
                break;
            default:
                return false;
        }
#endif

        return true;
    }

    template <typename TBody, typename TByteBuffer>
    inline static bool serialize_to_buffer(TBody *body, TByteBuffer *buf, SL_RPC_MESSAGE_ENCODE_TYPE encode_type = SL_RPC_MESSAGE_ENCODE_THRIFT)
    {
#ifdef SOCKETLITE_USE_THRIFT
        switch (encode_type)
        {
            case SL_RPC_MESSAGE_ENCODE_THRIFT:
                {
                    SL_Rpc_Thrift_BinaryProtocol<SL_ByteBuffer> thrift_protocol;
                    thrift_protocol.setBuffer(NULL, 0, buf);
                    body->write(&thrift_protocol);
                }
                break;
            case SL_RPC_MESSAGE_ENCODE_THRIFT_COMPACT_PROTOCOL:
                {
                    SL_Rpc_Thrift_CompactProtocol<SL_ByteBuffer> thrift_protocol;
                    thrift_protocol.setBuffer(NULL, 0, buf);
                    body->write(&thrift_protocol);
                }
                break;
            default:
                return false;
        }
#endif

        return true;
    }
};

//消息目标类型
typedef uint64 SL_Rpc_Message_DestinationType;
//消息目标集合类型
typedef std::set<SL_Rpc_Message_DestinationType> SL_Rpc_Message_Destinations;

template<typename THead, typename TBody, int message_type, typename TDestinationType = SL_Rpc_Message_DestinationType, typename TDestinations = std::set<TDestinationType> >
class SL_Rpc_Message
{
public:
    inline SL_Rpc_Message()
    {
        memset(&head_, 0, sizeof(THead));
    }

    inline ~SL_Rpc_Message()
    {
    }

    inline bool parse(char *data, int len, SL_RPC_MESSAGE_ENCODE_TYPE encode_type = SL_RPC_MESSAGE_ENCODE_THRIFT)
    { 
        memcpy(&head_, data, sizeof(THead));
        if (head_.flag & SL_RPC_MESSAGE_FLAG_LOCAL_BROADCAST)
        {
            TDestinationType dest_item;
            char *dest_pos = data + head_.length;
            char *data_end = data + len;
            while (dest_pos < data_end)
            {
                dest_item = THead::parse_destination( *(TDestinationType *)dest_pos );
                destinations_.insert(destinations_.end(), dest_item);
                dest_pos += sizeof(TDestinationType);
            }
        }
        else
        {
            destinations_.insert(destinations_.end(), head_.destination);
        }
        return SL_Rpc_MessageParser::parse_to_body<TBody>(&body_, data+sizeof(THead), len-sizeof(THead), encode_type);
    }

    template <typename TByteBuffer>
    inline bool serialize(TByteBuffer *buf, SL_RPC_MESSAGE_ENCODE_TYPE encode_type = SL_RPC_MESSAGE_ENCODE_THRIFT)
    {
        buf->reserve(sizeof(THead));
        buf->data_begin(buf->data_begin() + sizeof(THead));
        if (SL_Rpc_MessageParser::serialize_to_buffer<TBody, TByteBuffer>(&body_, buf, encode_type))
        {
            buf->data_begin(buf->data_begin() - sizeof(THead));
            head_.flag &= ~(SL_RPC_MESSAGE_FLAG_LOCAL_BROADCAST | SL_RPC_MESSAGE_FLAG_NOT_DESTINATIONS);
            if (0 == head_.type)
            {
                head_.type = MESSAGE_TYPE;
            }

            if (!destinations_.empty())
            {
                uint dest_size;
                if (head_.destination > 0)
                {
                    uint temp_size = destinations_.size();
                    destinations_.insert(destinations_.end(), head_.destination);
                    dest_size = destinations_.size();
                    if (temp_size < dest_size)
                    {
                        head_.flag |= SL_RPC_MESSAGE_FLAG_NOT_DESTINATIONS;
                    }

                    if (dest_size > 1)
                    {
                        int destination_index = 0;
                        TDestinationType dest_item;
                        typename TDestinations::iterator iter = destinations_.begin();
                        for (uint i = 0; i < dest_size; ++i)
                        {
                            if (*iter == head_.destination)
                            {
                                destination_index = i + 1;
                            }
                            dest_item = THead::serialize_destination( *iter );
                            buf->write((const char *)&dest_item, sizeof(TDestinationType));
                            ++iter;
                        }

                        head_.flag |= SL_RPC_MESSAGE_FLAG_LOCAL_BROADCAST;
                        head_.destination = buf->data_size() - sizeof(TDestinationType) * dest_size;
                        head_.length = buf->data_size();
                        memcpy(buf->data(), &head_, sizeof(THead));
                        THead::serialize((THead *)buf->data(), destination_index);
                        return true;
                    }
                }
                else
                {
                    dest_size = destinations_.size();
                    if (dest_size > 1)
                    {
                        TDestinationType dest_item;
                        typename TDestinations::iterator iter = destinations_.begin();
                        for (uint i = 0; i < dest_size; ++i)
                        {
                            dest_item = THead::serialize_destination( *iter );
                            buf->write((const char *)&dest_item, sizeof(TDestinationType));
                            ++iter;
                        }

                        head_.flag |= SL_RPC_MESSAGE_FLAG_LOCAL_BROADCAST;
                        head_.destination = buf->data_size() - sizeof(TDestinationType) * dest_size;
                        head_.length = buf->data_size();
                        memcpy(buf->data(), &head_, sizeof(THead));
                        THead::serialize((THead *)buf->data());
                        return true;
                    }
                    else if (dest_size == 1)
                    {
                        head_.destination = *destinations_.begin();
                    }
                }
            }
            else
            {
                if (head_.destination <= 0)
                {
                    head_.destination = head_.source;
                }
            }

            head_.length = buf->data_size();
            memcpy(buf->data(), &head_, sizeof(THead));
            THead::serialize((THead *)buf->data());
            return true;
        }

        buf->data_begin(buf->data_begin() - sizeof(THead));
        return false;
    }

    //过时被废弃(用于向后兼容). 新项目中请使用serialize
    template <typename TByteBuffer>
    inline bool serialize_ex(TByteBuffer *buf, SL_RPC_MESSAGE_ENCODE_TYPE encode_type = SL_RPC_MESSAGE_ENCODE_THRIFT)
    {
        return serialize(buf, encode_type);
    }

    //过时被废弃(用于向后兼容). 新项目中请使用serialize
    template <typename TByteBuffer>
    inline bool serialize_ex2(TByteBuffer *buf, SL_RPC_MESSAGE_ENCODE_TYPE encode_type = SL_RPC_MESSAGE_ENCODE_THRIFT)
    {
        return serialize(buf, encode_type);
    }

    //过时被废弃(用于向后兼容). 新项目中请使用serialize
    template <typename TByteBuffer>
    inline bool serialize_ex3(TByteBuffer *buf, SL_RPC_MESSAGE_ENCODE_TYPE encode_type = SL_RPC_MESSAGE_ENCODE_THRIFT)
    {
        return serialize(buf, encode_type);
    }

    inline void clear_destinations()
    {
        destinations_.clear();
    }

    inline void add_destination_item(const TDestinationType &dest_item)
    {
        destinations_.insert(destinations_.end(), dest_item);
    }

public:
    static const int MESSAGE_TYPE = message_type;

    THead           head_;          //消息头
    TBody           body_;          //消息体
    TDestinations   destinations_;  //消息目标列表(即消息的接收者id集合)
};

//rpc空消息体类, 用于适配SL_Rpc_Message
class SL_Rpc_NULL_MessageBody
{
public:
    inline SL_Rpc_NULL_MessageBody()
    {
    }
    inline ~SL_Rpc_NULL_MessageBody()
    {
    }

#ifdef SOCKETLITE_USE_THRIFT
    inline uint32_t read(::apache::thrift::protocol::TProtocol *iprot)
    {
        return 0;
    }

    inline uint32_t write(::apache::thrift::protocol::TProtocol *oprot) const
    {
        return 0;
    }
#endif
};

#endif

