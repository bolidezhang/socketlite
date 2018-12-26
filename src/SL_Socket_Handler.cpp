#include "SL_Socket_CommonAPI.h"
#include "SL_Socket_Source.h"
#include "SL_Socket_Handler.h"

#ifdef SOCKETLITE_OS_WINDOWS

int SL_Socket_Handler::handle_open(SL_SOCKET fd, SL_Socket_Source *socket_source, SL_Socket_Runner *socket_runner)
{
    socket_ = fd;
    socket_source_ = socket_source;
    socket_runner_ = socket_runner;
    last_errno_ = 0;
    current_status_ = STATUS_OPEN;
    next_status_ = STATUS_OPEN;
    SL_Socket_CommonAPI::socket_set_block(fd, false);

    per_io_data_.data_buffer.reserve(socket_source_->get_recvbuffer_size());
    per_io_data_.data_buffer.reset();
    per_io_data_.buffer.buf = per_io_data_.data_buffer.buffer();
    per_io_data_.buffer.len = per_io_data_.data_buffer.buffer_size();
    per_io_data_.oper_type = RECV_POSTED;
    return 0;
}

#else

int SL_Socket_Handler::handle_open(SL_SOCKET fd, SL_Socket_Source *socket_source, SL_Socket_Runner *socket_runner)
{
    socket_ = fd;
    socket_source_ = socket_source;
    socket_runner_ = socket_runner;
    last_errno_ = 0;
    current_status_ = STATUS_OPEN;
    next_status_ = STATUS_OPEN;
#ifndef SOCKETLITE_USE_ACCEPT4
    SL_Socket_CommonAPI::socket_set_block(fd, false);
#endif
    return 0;
}

#endif

int SL_Socket_Handler::write_data(const char *buf, int len)
{
    if (socket_source_->get_socket_handler() == this)
    {
        return socket_source_->send(buf, len, 0);
    }
    return SL_Socket_CommonAPI::socket_send(socket_, buf, len);
}

