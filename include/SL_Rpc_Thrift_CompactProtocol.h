#ifndef SOCKETLITE_RPC_THRIFT_COMPACT_PROTOCOL_H
#define SOCKETLITE_RPC_THRIFT_COMPACT_PROTOCOL_H
#ifdef SOCKETLITE_USE_THRIFT

#include <limits>
#include <stack>
#include <boost/shared_ptr.hpp>
#include <protocol/TProtocol.h>
#include "SL_ByteBuffer.h"

/**
 * C++ Implementation of the Compact Protocol as described in THRIFT-110
 * varint encode/decode
 */

#ifdef __GNUC__
#define UNLIKELY(val) (__builtin_expect((val), 0))
#else
#define UNLIKELY(val) (val)
#endif

/*
 * TCompactProtocol::i*ToZigzag depend on the fact that the right shift
 * operator on a signed integer is an arithmetic (sign-extending) shift.
 * If this is not the case, the current implementation will not work.
 * If anyone encounters this error, we can try to figure out the best
 * way to implement an arithmetic right shift on their platform.
 */
//#if !defined(SIGNED_RIGHT_SHIFT_IS) || !defined(ARITHMETIC_RIGHT_SHIFT)
//# error "Unable to determine the behavior of a signed right shift"
//#endif
//#if SIGNED_RIGHT_SHIFT_IS != ARITHMETIC_RIGHT_SHIFT
//# error "TCompactProtocol currenly only works if a signed right shift is arithmetic"
//#endif

using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

struct SL_Rpc_Thrift_CompactProtocol_Internal
{
    enum Types 
    {
        CT_STOP           = 0x00,
        CT_BOOLEAN_TRUE   = 0x01,
        CT_BOOLEAN_FALSE  = 0x02,
        CT_BYTE           = 0x03,
        CT_I16            = 0x04,
        CT_I32            = 0x05,
        CT_I64            = 0x06,
        CT_DOUBLE         = 0x07,
        CT_BINARY         = 0x08,
        CT_LIST           = 0x09,
        CT_SET            = 0x0A,
        CT_MAP            = 0x0B,
        CT_STRUCT         = 0x0C,
    };

    static const int8_t TTypeToCType[16];
};

template <typename TByteBuffer>
class SL_Rpc_Thrift_CompactProtocol : public apache::thrift::protocol::TProtocol
{
protected:
    static const int8_t  PROTOCOL_ID = 0x82;
    static const int8_t  VERSION_N = 1;
    static const int8_t  VERSION_MASK = 0x1f;   // 0001 1111
    static const int8_t  TYPE_MASK = 0xE0;      // 1110 0000
    static const int32_t TYPE_SHIFT_AMOUNT = 5;

    /**
    * (Writing) If we encounter a boolean field begin, save the TField here
    * so it can have the value incorporated.
    */
    struct
    {
        const char *name;
        TType   fieldType;
        int16_t fieldId;
    } booleanField_;

    /**
    * (Reading) If we read a field header, and it's a boolean field, save
    * the boolean value here so that readBool can use it.
    */
    struct 
    {
        bool hasBoolValue;
        bool boolValue;
    } boolValue_;

    /**
    * Used to keep track of the last field for the current and previous structs,
    * so we can do the delta stuff.
    */

    std::stack<int16_t> lastField_;
    int16_t lastFieldId_;

public:
    inline SL_Rpc_Thrift_CompactProtocol()
        : lastFieldId_(0)
        , write_buffer_(NULL)
        , read_buffer_(NULL)
        , read_buffer_len_(0)
    {
        booleanField_.name = NULL;
        boolValue_.hasBoolValue = false;
    }

    inline virtual ~SL_Rpc_Thrift_CompactProtocol()
    {
    }

    inline void setBuffer(char *read_buffer, int read_buffer_len, TByteBuffer *write_buffer)
    {
        write_buffer_    = write_buffer;
        read_buffer_     = read_buffer;
        read_buffer_len_ = read_buffer_len;
    }

    /**
    * Writing functions
    */
    virtual uint32_t writeMessageBegin(const std::string &name, const TMessageType messageType, const int32_t seqid)
    {
        uint32_t wsize = writeByte(PROTOCOL_ID);
        wsize += writeByte((VERSION_N & VERSION_MASK) | (((int32_t)messageType << TYPE_SHIFT_AMOUNT) & TYPE_MASK));
        wsize += writeVarint32(seqid);
        wsize += writeString(name);
        return wsize;
    }

    inline uint32_t writeStructBegin(const char *name)
    {
        lastField_.push(lastFieldId_);
        lastFieldId_ = 0;
        return 0;        
    }

    inline uint32_t writeStructEnd()
    {
        lastFieldId_ = lastField_.top();
        lastField_.pop();
        return 0;
    }

    inline uint32_t writeFieldBegin(const char *name, const TType fieldType, const int16_t fieldId)
    {
        if (fieldType == T_BOOL)
        {
            booleanField_.name = name;
            booleanField_.fieldType = fieldType;
            booleanField_.fieldId = fieldId;
        } 
        else 
        {
            return writeFieldBeginInternal(name, fieldType, fieldId, -1);
        }
        return 0;
    }

    inline uint32_t writeFieldStop()
    {
        return writeByte(T_STOP);
    }

    inline uint32_t writeListBegin(const TType elemType, const uint32_t size)
    {
        return writeCollectionBegin(elemType, size);
    }

    inline uint32_t writeSetBegin(const TType elemType, const uint32_t size)
    {
        return writeCollectionBegin(elemType, size);
    }

    virtual uint32_t writeMapBegin(const TType keyType, const TType valType, const uint32_t size)
    {
        uint32_t wsize;
        if (size == 0) 
        {
            wsize = writeByte(0);
        } 
        else 
        {
            wsize = writeVarint32(size);
            wsize += writeByte(getCompactType(keyType) << 4 | getCompactType(valType));
        }
        return wsize;
    }

    inline uint32_t writeBool(const bool value)
    {
        uint32_t wsize;
        if (booleanField_.name != NULL) 
        {
            // we haven't written the field header yet
            wsize = writeFieldBeginInternal( booleanField_.name, 
                booleanField_.fieldType, 
                booleanField_.fieldId, 
                (value ? SL_Rpc_Thrift_CompactProtocol_Internal::CT_BOOLEAN_TRUE : SL_Rpc_Thrift_CompactProtocol_Internal::CT_BOOLEAN_FALSE) );
            booleanField_.name = NULL;
        } 
        else 
        {
            // we're not part of a field, so just write the value
            wsize = writeByte( (value ? SL_Rpc_Thrift_CompactProtocol_Internal::CT_BOOLEAN_TRUE : SL_Rpc_Thrift_CompactProtocol_Internal::CT_BOOLEAN_FALSE) );
        }
        return wsize;
    }

    inline uint32_t writeByte(const int8_t byte)
    {
        write_buffer_->write((const char *)&byte, 1);
        return 1;
    }

    inline uint32_t writeI16(const int16_t i16)
    {
        return writeVarint32(i32ToZigzag(i16));
    }

    inline uint32_t writeI32(const int32_t i32)
    {
        return writeVarint32(i32ToZigzag(i32));
    }

    inline uint32_t writeI64(const int64_t i64)
    {
        return writeVarint64(i64ToZigzag(i64));
    }

    inline uint32_t writeDouble(const double dub)
    {
        BOOST_STATIC_ASSERT(sizeof(double) == sizeof(uint64_t));
        BOOST_STATIC_ASSERT(std::numeric_limits<double>::is_iec559);

        uint64_t bits = bitwise_cast<uint64_t>(dub);
        bits = htolell(bits);
        write_buffer_->write((const char *)&bits, 8);
        return 8;
    }

    inline uint32_t writeString(const std::string& str)
    {
        return writeBinary(str);
    }

    inline uint32_t writeBinary(const std::string& str)
    {
        uint32_t ssize = str.size();
        uint32_t wsize = writeVarint32(ssize) + ssize;
        write_buffer_->write((const char *)str.data(), ssize);
        return wsize;
    }

    /**
    * These methods are called by structs, but don't actually have any wired
    * output or purpose
    */
    inline virtual uint32_t writeMessageEnd() { return 0; }
    inline uint32_t writeMapEnd() { return 0; }
    inline uint32_t writeListEnd() { return 0; }
    inline uint32_t writeSetEnd() { return 0; }
    inline uint32_t writeFieldEnd() { return 0; }

protected:
    inline int32_t writeFieldBeginInternal(const char* name, const TType fieldType, const int16_t fieldId, int8_t typeOverride)
    {
        uint32_t wsize;

        // if there's a type override, use that.
        int8_t typeToWrite = (typeOverride == -1 ? getCompactType(fieldType) : typeOverride);

        // check if we can use delta encoding for the field id
        if (fieldId > lastFieldId_ && fieldId - lastFieldId_ <= 15) 
        {
            // write them together
            wsize  = writeByte((fieldId - lastFieldId_) << 4 | typeToWrite);
        } 
        else 
        {
            // write them separate
            wsize  = writeByte(typeToWrite);
            wsize += writeI16(fieldId);
        }

        lastFieldId_ = fieldId;
        return wsize;
    }

    inline uint32_t writeCollectionBegin(int8_t elemType, int32_t size)
    {
        uint32_t wsize;
        if (size <= 14) 
        {
            wsize  = writeByte(size << 4 | getCompactType(elemType));
        } 
        else 
        {
            wsize  = writeByte(0xf0 | getCompactType(elemType));
            wsize += writeVarint32(size);
        }
        return wsize;
    }

    /**
     * Write an i32 as a varint. Results in 1-5 bytes on the wire.
     */
    inline uint32_t writeVarint32(uint32_t n)
    {
        if (write_buffer_->free_size() < 5)
        {
            write_buffer_->reserve(2 * write_buffer_->buffer_size() + 10);
        }
        uint8_t  *buf  = (uint8_t *)(write_buffer_->data() + write_buffer_->data_size());
        uint32_t wsize = 1;

        //方法1: leveldb varint32 encode implementation
        static const int B = 0x80;
        if (n < (1<<7))
        {
            buf[0] = n;
        }
        else if (n < (1<<14))
        {
            buf[0] = n | B;
            buf[1] = n>>7;
            wsize  = 2;
        }
        else if (n < (1<<21))
        {
            buf[0] = n | B;
            buf[1] = (n>>7) | B;
            buf[2] = n>>14;
            wsize  = 3;
        }
        else if (n < (1<<28))
        {
            buf[0] = n | B;
            buf[1] = (n>>7) | B;
            buf[2] = (n>>14) | B;
            buf[3] = n>>21;
            wsize  = 4;
        }
        else
        {
            buf[0] = n | B;
            buf[1] = (n>>7) | B;
            buf[2] = (n>>14) | B;
            buf[3] = (n>>21) | B;
            buf[4] = n>>28;
            wsize  = 5;
        }
        write_buffer_->advance(wsize);
        return wsize;

        ////方法2: protobuf varint32 encode implementation
        //buf[0] = static_cast<uint8>(n | 0x80);
        //if (n >= (1 << 7)) 
        //{
        //    buf[1] = static_cast<uint8_t>((n >> 7) | 0x80);
        //    if (n >= (1 << 14)) 
        //    {
        //        buf[2] = static_cast<uint8_t>((n >> 14) | 0x80);
        //        if (n >= (1 << 21)) 
        //        {
        //            buf[3] = static_cast<uint8_t>((n >> 21) | 0x80);
        //            if (n >= (1 << 28)) 
        //            {
        //                buf[4] = static_cast<uint8_t>(n >> 28);
        //                wsize = 5;
        //            } 
        //            else 
        //            {
        //                buf[3] &= 0x7F;
        //                wsize = 4;
        //            }
        //        } 
        //        else 
        //        {
        //            buf[2] &= 0x7F;
        //            wsize = 3;
        //        }
        //    } 
        //    else 
        //    {
        //        buf[1] &= 0x7F;
        //        wsize = 2;
        //    }
        //} 
        //else 
        //{
        //    buf[0] &= 0x7F;
        //}
        //write_buffer_->advance(wsize);
        //return wsize;

        ////方法三: varint encode common implementation
        //wsize = 0;
        //while (n > 0x7F)
        //{
        //    buf[wsize++] = (static_cast<uint8_t>(n) & 0x7F) | 0x80;
        //    n >>= 7;
        //}
        //buf[wsize++] = static_cast<uint8_t>(n);
        //write_buffer_->advance(wsize);
        //return wsize;
    }

    /**
     * Write an i64 as a varint. Results in 1-10 bytes on the wire.
     */
    inline uint32_t writeVarint64(uint64_t n)
    {
        if (write_buffer_->free_size() < 10)
        {
            write_buffer_->reserve(2 * write_buffer_->buffer_size() + 20);
        }
        uint8_t  *buf  = (uint8_t *)(write_buffer_->data() + write_buffer_->data_size());
        uint32_t wsize;

        //方法1: protobuf varint64 encode implementation
        // Splitting into 32-bit pieces gives better performance on 32-bit processors.
        uint32_t part0 = static_cast<uint32_t>(n      );
        uint32_t part1 = static_cast<uint32_t>(n >> 28);
        uint32_t part2 = static_cast<uint32_t>(n >> 56);

        // Here we can't really optimize for small numbers, since the value is
        // split into three parts.  Cheking for numbers < 128, for instance,
        // would require three comparisons, since you'd have to make sure part1
        // and part2 are zero.  However, if the caller is using 64-bit integers,
        // it is likely that they expect the numbers to often be very large, so
        // we probably don't want to optimize for small numbers anyway.  Thus,
        // we end up with a hardcoded binary search tree...
        if (part2 == 0) 
        {
            if (part1 == 0) 
            {
                if (part0 < (1 << 14)) 
                {
                    if (part0 < (1 << 7)) 
                    {
                        wsize = 1;
                        goto size1;
                    } 
                    else 
                    {
                        wsize = 2;
                        goto size2;
                    }
                } 
                else 
                {
                    if (part0 < (1 << 21)) 
                    {
                        wsize = 3; 
                        goto size3;
                    } 
                    else 
                    {
                        wsize = 4; 
                        goto size4;
                    }
                }
            } 
            else 
            {
                if (part1 < (1 << 14)) 
                {
                    if (part1 < (1 << 7)) 
                    {
                        wsize = 5; 
                        goto size5;
                    } 
                    else 
                    {
                        wsize = 6; 
                        goto size6;
                    }
                } 
                else 
                {
                    if (part1 < (1 << 21)) 
                    {
                        wsize = 7; 
                        goto size7;
                    } 
                    else 
                    {
                        wsize = 8; 
                        goto size8;
                    }
                }
            }
        } 
        else 
        {
            if (part2 < (1 << 7)) 
            {
                wsize = 9; 
                goto size9;
            } 
            else 
            {
                wsize = 10; 
                goto size10;
            }
        }

        size10: buf[9] = static_cast<uint8_t>((part2 >>  7) | 0x80);
        size9 : buf[8] = static_cast<uint8_t>((part2      ) | 0x80);
        size8 : buf[7] = static_cast<uint8_t>((part1 >> 21) | 0x80);
        size7 : buf[6] = static_cast<uint8_t>((part1 >> 14) | 0x80);
        size6 : buf[5] = static_cast<uint8_t>((part1 >>  7) | 0x80);
        size5 : buf[4] = static_cast<uint8_t>((part1      ) | 0x80);
        size4 : buf[3] = static_cast<uint8_t>((part0 >> 21) | 0x80);
        size3 : buf[2] = static_cast<uint8_t>((part0 >> 14) | 0x80);
        size2 : buf[1] = static_cast<uint8_t>((part0 >>  7) | 0x80);
        size1 : buf[0] = static_cast<uint8_t>((part0      ) | 0x80);
        buf[wsize-1] &= 0x7F;

        write_buffer_->advance(wsize);
        return wsize;

        ////方法2: varint encode common implementation
        //wsize = 0;
        //while (n > 0x7F)
        //{
        //    buf[wsize++] = (static_cast<uint8_t>(n) & 0x7F) | 0x80;
        //    n >>= 7;
        //}
        //buf[wsize++] = static_cast<uint8_t>(n);
        //write_buffer_->advance(wsize);
        //return wsize;
    }

    /**
     * Convert n into a zigzag int. This allows negative numbers to be
     * represented compactly as a varint.
     */
    inline uint32_t i32ToZigzag(const int32_t n)
    {
        return (n << 1) ^ (n >> 31);
    }

    /**
     * Convert l into a zigzag long. This allows negative numbers to be
     * represented compactly as a varint.
     */
    inline uint64_t i64ToZigzag(const int64_t l)
    {
        return (l << 1) ^ (l >> 63);
    }

    /**
     * Given a TType value, find the appropriate TCompactProtocol.Type value
     */
    inline int8_t getCompactType(int8_t ttype)
    {
        return SL_Rpc_Thrift_CompactProtocol_Internal::TTypeToCType[ttype];
    }

public:
    inline uint32_t readMessageBegin(std::string& name, TMessageType& messageType, int32_t& seqid)
    {
        int8_t protocolId;
        int8_t versionAndType;
        int8_t version;

        uint32_t rsize = readByte(protocolId);
        if (protocolId != PROTOCOL_ID) 
        {
            throw TProtocolException(TProtocolException::BAD_VERSION, "Bad protocol identifier");
        }

        rsize += readByte(versionAndType);
        version = (int8_t)(versionAndType & VERSION_MASK);
        if (version != VERSION_N) 
        {
            throw TProtocolException(TProtocolException::BAD_VERSION, "Bad protocol version");
        }

        messageType = (TMessageType)((versionAndType >> TYPE_SHIFT_AMOUNT) & 0x03);
        rsize += readVarint32(seqid);
        rsize += readString(name);

        return rsize;
    }

    inline uint32_t readStructBegin(std::string& name)
    {
        name.clear();
        lastField_.push(lastFieldId_);
        lastFieldId_ = 0;
        return 0;
    }

    inline uint32_t readStructEnd()
    {
        lastFieldId_ = lastField_.top();
        lastField_.pop();
        return 0;
    }

    inline uint32_t readFieldBegin(std::string& name, TType& fieldType, int16_t& fieldId)
    {
        int8_t   byte;
        int8_t   type;

        uint32_t rsize = readByte(byte);
        type = (byte & 0x0f);

        // if it's a stop, then we can return immediately, as the struct is over.
        if (type == T_STOP) 
        {
            fieldType = T_STOP;
            fieldId = 0;
            return rsize;
        }

        // mask off the 4 MSB of the type header. it could contain a field id delta.
        int16_t modifier = (int16_t)(((uint8_t)byte & 0xf0) >> 4);
        if (modifier == 0) 
        {
            // not a delta, look ahead for the zigzag varint field id.
            rsize += readI16(fieldId);
        } 
        else 
        {
            fieldId = (int16_t)(lastFieldId_ + modifier);
        }
        fieldType = getTType(type);

        // if this happens to be a boolean field, the value is encoded in the type
        if (type == SL_Rpc_Thrift_CompactProtocol_Internal::CT_BOOLEAN_TRUE || type == SL_Rpc_Thrift_CompactProtocol_Internal::CT_BOOLEAN_FALSE) 
        {
            // save the boolean value in a special instance variable.
            boolValue_.hasBoolValue = true;
            boolValue_.boolValue = (type == SL_Rpc_Thrift_CompactProtocol_Internal::CT_BOOLEAN_TRUE ? true : false);
        }

        // push the new field onto the field stack so we can keep the deltas going.
        lastFieldId_ = fieldId;
        return rsize;
    }

    inline uint32_t readMapBegin(TType& keyType, TType& valType, uint32_t& size)
    {
        int8_t   kvType = 0;
        int32_t  msize = 0;

        uint32_t rsize = readVarint32(msize);
        if (msize > 0)
        {
            rsize  += readByte(kvType);
            keyType = getTType((int8_t)((uint8_t)kvType >> 4));
            valType = getTType((int8_t)((uint8_t)kvType & 0xf));
            size = (uint32_t)msize;
            return rsize;
        }
        else if (msize == 0)
        {
            keyType = getTType(0);
            valType = getTType(0);
            size = (uint32_t)msize;
            return rsize;
        }
        else
        {//msize < 0
            throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
            return 0;
        }
    }

    inline uint32_t readListBegin(TType& elemType, uint32_t& size)
    {
        int8_t   size_and_type;
        int32_t  lsize;

        uint32_t rsize = readByte(size_and_type);
        lsize = ((uint8_t)size_and_type >> 4) & 0x0f;
        if (lsize == 15)
        {
            rsize += readVarint32(lsize);
        }

        if (lsize >= 0)
        {
            elemType = getTType((int8_t)(size_and_type & 0x0f));
            size = (uint32_t)lsize;
            return rsize;
        }
        else
        {
            throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
            return 0;
        }
    }

    inline uint32_t readSetBegin(TType& elemType, uint32_t& size)
    {
        return readListBegin(elemType, size);
    }

    /**
     * Read a boolean off the wire. If this is a boolean field, the value should
     * already have been read during readFieldBegin, so we'll just consume the
     * pre-stored value. Otherwise, read a byte.
     */
    inline uint32_t readBool(bool& value)
    {
        if (boolValue_.hasBoolValue == true) 
        {
            value = boolValue_.boolValue;
            boolValue_.hasBoolValue = false;
            return 0;
        } 
        else 
        {
            int8_t val;
            readByte(val);
            value = (val == SL_Rpc_Thrift_CompactProtocol_Internal::CT_BOOLEAN_TRUE);
            return 1;
        }
    }

    /**
     * Read a single byte off the wire. Nothing interesting here.
     */
    inline uint32_t readByte(int8_t& byte)
    {
        char *b = readBuffer(1);
        byte = *(int8_t *)b;
        return 1;
    }

    /**
     * Read an i16 from the wire as a zigzag varint.
     */
    inline uint32_t readI16(int16_t& i16)
    {
        int32_t  value;
        uint32_t rsize = readVarint32(value);
        i16 = (int16_t)zigzagToI32(value);
        return rsize;
    }

    /**
     * Read an i32 from the wire as a zigzag varint.
     */
    inline uint32_t readI32(int32_t& i32)
    {
        int32_t  value;
        uint32_t rsize = readVarint32(value);
        i32 = zigzagToI32(value);
        return rsize;
    }

    /**
     * Read an i64 from the wire as a zigzag varint.
     */
    inline uint32_t readI64(int64_t& i64)
    {
        int64_t  value;
        uint32_t rsize = readVarint64(value);
        i64 = zigzagToI64(value);
        return rsize;
    }

    /**
     * No magic here - just read a double off the wire.
     */
    inline uint32_t readDouble(double& dub)
    {
        BOOST_STATIC_ASSERT(sizeof(double) == sizeof(uint64_t));
        BOOST_STATIC_ASSERT(std::numeric_limits<double>::is_iec559);

        char *b = readBuffer(8);
        uint64_t bits = *(uint64_t *)b;
        bits = letohll(bits);
        dub  = bitwise_cast<double>(bits);
        return 8;
    }

    inline uint32_t readString(std::string& str)
    {
        return readBinary(str);
    }

    /**
     * Read a byte[] from the wire.
     */
    inline uint32_t readBinary(std::string& str)
    {
        int32_t size;
        int32_t rsize = readVarint32(size);
        if (size > 0)
        {
            char *b = readBuffer(size);
            str.assign(b, size);
            return rsize + (uint32_t)size;
        }
        else if (size < 0)
        {// Catch error cases
            throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
            return 0;
        }
        else
        {// size == 0, Catch empty string case
            str.clear();
            return rsize;
        }
    }

    /*
    *These methods are here for the struct to call, but don't have any wire
    * encoding.
    */
    inline uint32_t readMessageEnd() { return 0; }
    inline uint32_t readFieldEnd() { return 0; }
    inline uint32_t readMapEnd() { return 0; }
    inline uint32_t readListEnd() { return 0; }
    inline uint32_t readSetEnd() { return 0; }

protected:
    /**
     * Read an i32 from the wire as a varint. The MSB of each byte is set
     * if there is another byte to follow. This can read up to 5 bytes.
     */
    inline uint32_t readVarint32(int32_t& i32)
    {
        #define STEP0 do {                  \
            b    = borrowed[0];             \
            val  = (b & 0x7F);              \
            if (!(b & 0x80))                \
            {                               \
                goto done;                  \
            }                               \
        } while (0)

        #define STEP(n) do {                \
            b    = borrowed[n];             \
            val |= (b & 0x7F) << (7 * n);   \
            if (!(b & 0x80))                \
            {                               \
                rsize = n + 1;              \
                goto done;                  \
            }                               \
        } while (0)

        int i32_size = 5;
        const uint8_t *borrowed = (const uint8_t *)borrowBuffer(&i32_size);
        uint32_t b;
        uint32_t val = 0;
        int rsize = 1;

        //方法1: protobuf varint32 decode implementation
        switch (i32_size)
        {
            case 5:
                {
                    STEP0; STEP(1); STEP(2); STEP(3); STEP(4);
                }
                break;
            case 4:
                {
                    STEP0; STEP(1); STEP(2); STEP(3);
                }
                break;
            case 3:
                {
                    STEP0; STEP(1); STEP(2);
                }
                break;
            case 2:
                {
                    STEP0; STEP(1);
                }
                break;
            case 1:
                {
                    STEP0;
                }
                break;
            default:
                break;
        }

        throw TProtocolException(TProtocolException::INVALID_DATA, "Variable-length int32 over 5 bytes.");
        return 0;

    done:
        i32 = val;
        consumeBuffer(rsize);
        return rsize;

        //方法2: varint decode common implementation
        //rsize = 0;
        //int shift = 0;
        //while (rsize < i32_size)
        //{
        //    b      = borrowed[rsize];
        //    val   |= (b & 0x7f) << shift;
        //    shift += 7;

        //    ++rsize;
        //    if (!(b & 0x80))
        //    {
        //        i32 = val;
        //        consumeBuffer(rsize);
        //        return rsize;
        //    }
        //}
        //throw TProtocolException(TProtocolException::INVALID_DATA, "Variable-length int32 over 5 bytes.");
        //return 0;
    }

    /**
     * Read an i64 from the wire as a proper varint. The MSB of each byte is set
     * if there is another byte to follow. This can read up to 10 bytes.
     */
    inline uint32_t readVarint64(int64_t& i64)
    {
        #define STEP0 do {                  \
            b    = borrowed[0];             \
            val  = (b & 0x7F);              \
            if (!(b & 0x80))                \
            {                               \
                goto done;                  \
            }                               \
        } while (0)

        #define STEP(n) do {                \
            b    = borrowed[n];             \
            val |= (b & 0x7F) << (7 * n);   \
            if (!(b & 0x80))                \
            {                               \
                rsize = n + 1;              \
                goto done;                  \
            }                               \
        } while (0)

        int i64_size = 10;
        const uint8_t *borrowed = (const uint8_t *)borrowBuffer(&i64_size);
        uint64_t val = 0;
        int      rsize = 1;
        uint64_t b;

        //protobuf varint64 decode implementation
        switch (i64_size)
        {
            case 10:
                {
                    uint32_t b2;

                    // Splitting into 32-bit pieces gives better performance on 32-bit processors.
                    uint32_t part0 = 0;
                    uint32_t part1 = 0;
                    uint32_t part2 = 0;

                    b2     = borrowed[0]; 
                    part0  = (b2 & 0x7F);
                    if (!(b2 & 0x80))
                    {
                        goto done10;
                    }

                    b2     = borrowed[1]; 
                    part0 |= (b2 & 0x7F) <<  7;
                    if (!(b2 & 0x80)) 
                    {
                        rsize = 2;
                        goto done10;
                    }

                    b2     = borrowed[2]; 
                    part0 |= (b2 & 0x7F) << 14; 
                    if (!(b2 & 0x80)) 
                    {
                        rsize = 3;
                        goto done10;
                    }

                    b2     = borrowed[3];
                    part0 |= (b2 & 0x7F) << 21; 
                    if (!(b2 & 0x80)) 
                    {
                        rsize = 4;
                        goto done10;
                    }

                    b2     = borrowed[4];
                    part1  = (b2 & 0x7F)      ; 
                    if (!(b2 & 0x80)) 
                    {
                        rsize = 5;
                        goto done10;
                    }

                    b2     = borrowed[5]; 
                    part1 |= (b2 & 0x7F) <<  7; 
                    if (!(b2 & 0x80)) 
                    {
                        rsize = 6;
                        goto done10;
                    }

                    b2     = borrowed[6];
                    part1 |= (b2 & 0x7F) << 14;
                    if (!(b2 & 0x80)) 
                    {
                        rsize = 7;
                        goto done10;
                    }

                    b2     = borrowed[7];
                    part1 |= (b2 & 0x7F) << 21; 
                    if (!(b2 & 0x80)) 
                    {
                        rsize = 8;
                        goto done10;
                    }

                    b2     = borrowed[8]; 
                    part2  = (b2 & 0x7F);
                    if (!(b2 & 0x80)) 
                    {
                        rsize = 9;
                        goto done10;
                    }

                    b2     = borrowed[9];
                    part2 |= (b2 & 0x7F) <<  7;
                    if (!(b2 & 0x80))
                    {
                        rsize = 10;
                        goto done10;
                    }

                    // We have overrun the maximum size of a varint (10 bytes).  The data
                    // must be corrupt.
                    throw TProtocolException(TProtocolException::INVALID_DATA, "Variable-length int64 over 10 bytes.");
                    return 0;

                done10:
                    i64 = (static_cast<uint64_t>(part0)) |(static_cast<uint64_t>(part1) << 28) | (static_cast<uint64_t>(part2) << 56);
                    consumeBuffer(rsize);
                    return rsize;
                }
                break;
            case 9:
                {
                    STEP0; STEP(1); STEP(2); STEP(3); STEP(4); STEP(5); STEP(6); STEP(7); STEP(8);
                }
                break;
            case 8:
                {
                    STEP0; STEP(1); STEP(2); STEP(3); STEP(4); STEP(5); STEP(6); STEP(7);
                }
                break;
            case 7:
                {
                    STEP0; STEP(1); STEP(2); STEP(3); STEP(4); STEP(5); STEP(6); STEP(7);
                }
                break;
            case 6:
                {
                    STEP0; STEP(1); STEP(2); STEP(3); STEP(4); STEP(5);
                }
                break;
            case 5:
                {
                    STEP0; STEP(1); STEP(2); STEP(3); STEP(4);
                }
                break;
            case 4:
                {
                    STEP0; STEP(1); STEP(2); STEP(3);
                }
                break;
            case 3:
                {
                    STEP0; STEP(1); STEP(2);
                }
                break;
            case 2:
                {
                    STEP0; STEP(1);
                }
                break;
            case 1:
                {
                    STEP0;
                }
                break;
            default:
                break;
        }

        throw TProtocolException(TProtocolException::INVALID_DATA, "Variable-length int64 over 10 bytes.");
        return 0;

    done:
        i64 = val;
        consumeBuffer(rsize);
        return rsize;

        ////方法2: varint decode common implementation
        //    int shift = 0;
        //    rsize = 0;
        //    while (rsize < i64_size)
        //    {
        //        b      = borrowed[rsize];
        //        val   |= (b & 0x7f) << shift;
        //        shift += 7;

        //        ++rsize;
        //        if (!(b & 0x80))
        //        {
        //            i64 = val;
        //            consumeBuffer(rsize);
        //            return rsize;
        //        }
        //    }
        //    throw TProtocolException(TProtocolException::INVALID_DATA, "Variable-length int64 over 10 bytes.");
        //    return 0;
    }

    inline char* readBuffer(int len)
    {
        if (read_buffer_len_ >= len)
        {
            char *ret = read_buffer_;
            read_buffer_     += len;
            read_buffer_len_ -= len;
            return ret;
        }
        else
        {
            throw TProtocolException(TProtocolException::INVALID_DATA);
        }
        return NULL;
    }
    
    inline char* borrowBuffer(int *len)
    {
        if (read_buffer_len_ >= *len)
        {
            return read_buffer_;
        }
        else
        {
            if (read_buffer_len_ > 0)
            {
                *len = read_buffer_len_;
                return read_buffer_;
            }
            else
            {
                throw TProtocolException(TProtocolException::INVALID_DATA);
            }
            return NULL;
        }
    }

    inline void consumeBuffer(int len)
    {
        read_buffer_     += len;
        read_buffer_len_ -= len;
    }

    /**
     * Convert from zigzag int to int.
     */
    inline int32_t zigzagToI32(uint32_t n)
    {
        return (n >> 1) ^ -(n & 1);
    }

    inline int64_t zigzagToI64(uint64_t n)
    {
        return (n >> 1) ^ -(n & 1);
    }

    inline TType getTType(int8_t type)
    {
        switch (type)
        {
            case T_STOP:
                return T_STOP;
            case SL_Rpc_Thrift_CompactProtocol_Internal::CT_BOOLEAN_FALSE:
            case SL_Rpc_Thrift_CompactProtocol_Internal::CT_BOOLEAN_TRUE:
                return T_BOOL;
            case SL_Rpc_Thrift_CompactProtocol_Internal::CT_BYTE:
                return T_BYTE;
            case SL_Rpc_Thrift_CompactProtocol_Internal::CT_I16:
                return T_I16;
            case SL_Rpc_Thrift_CompactProtocol_Internal::CT_I32:
                return T_I32;
            case SL_Rpc_Thrift_CompactProtocol_Internal::CT_I64:
                return T_I64;
            case SL_Rpc_Thrift_CompactProtocol_Internal::CT_DOUBLE:
                return T_DOUBLE;
            case SL_Rpc_Thrift_CompactProtocol_Internal::CT_BINARY:
                return T_STRING;
            case SL_Rpc_Thrift_CompactProtocol_Internal::CT_LIST:
                return T_LIST;
            case SL_Rpc_Thrift_CompactProtocol_Internal::CT_SET:
                return T_SET;
            case SL_Rpc_Thrift_CompactProtocol_Internal::CT_MAP:
                return T_MAP;
            case SL_Rpc_Thrift_CompactProtocol_Internal::CT_STRUCT:
                return T_STRUCT;
            default:
                throw TException("don't know what type: " + type);
        }
        return T_STOP;
    }

protected:
    //写缓冲区
    TByteBuffer *write_buffer_;

    //读缓冲区
    char *read_buffer_;     //读缓冲区
    int  read_buffer_len_;  //读缓冲区长度
};

#endif  //#ifdef SOCKETLITE_USE_THRIFT
#endif  //#ifndef SOCKETLITE_RPC_THRIFT_COMPACT_PROTOCOL_H

