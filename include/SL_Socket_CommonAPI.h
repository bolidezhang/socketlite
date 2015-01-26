#ifndef SOCKETLITE_SOCKET_COMMON_API_H
#define SOCKETLITE_SOCKET_COMMON_API_H

#include <time.h>
#include "SL_Config.h"
#include "SL_OS_API.h"
#include "SL_Utility_Memory.h"
#ifdef SOCKETLITE_OS_WINDOWS
    #include <io.h>
    #include "Shlwapi.h"
    #pragma comment(lib, "shlwapi.lib")

    /* timeb.h is actually xsi legacy functionality */
    #include <sys/timeb.h>

    //windows多媒体库(主要使用其中timeGetTime())
    #include <MMSystem.h>
    #pragma comment(lib, "Winmm.lib")

    //windows视频库VFW
    //#include <Vfw.h>
    //#pragma comment(lib, "Vfw32.lib")
#else
    #include <sys/time.h>
    #include <sys/ioctl.h>
    #include <byteswap.h>
#endif

class SOCKETLITE_API SL_Socket_CommonAPI
{
private:
    SL_Socket_CommonAPI()
    {
    }

    ~SL_Socket_CommonAPI()
    {
    }
    
public:
    //socket_init 参数timer_resolution含义： 1) =0 自动设置OS允许定时精度的最小值 2) >0手动设置定时精度(精度范围: 1-5 ms)
    static int socket_init(int version_high, int version_low, int timer_resolution_ms = 0)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            //初始化系统高精度计数器的频率
            system_counter_frequency_ = util_system_counter_frequency();

            // initialize timer resolution
            // attempt to set the resolution as low as possible from 1ms to 5ms
            if (timer_resolution_ms <= 0)
            {//自动设置OS允许定时精度的最小值
                for (unsigned int i=1; i<=5; ++i)
                {
                    if (timeBeginPeriod(i) == TIMERR_NOERROR)
                    {
                        timer_resolution_ms_ = i;
                        break;
                    }
                }
            }
            else
            {
                if (timer_resolution_ms > 5)
                {
                    timer_resolution_ms = 5;
                }
                if (timeBeginPeriod(timer_resolution_ms) == TIMERR_NOERROR)
                {
                    timer_resolution_ms_ = timer_resolution_ms;
                }
            }

            //initialize winsock
            WSADATA wsaData;
            WORD version_requested = MAKEWORD(version_high, version_low);
            return WSAStartup(version_requested, &wsaData);

        #else
            return 0;
        #endif
    }

    static int socket_fini()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            // shutdown timer
            if (timer_resolution_ms_ > 0)
            {
                timeEndPeriod(timer_resolution_ms_);
            }

            return WSACleanup();
        #else
            return 0;
        #endif
    }

    static inline int socket_get_lasterror()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            return WSAGetLastError();
        #else
            return errno;
        #endif
    }

    static inline int socket_get_lasterror_wsa()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            return WSAGetLastError();
        #else
            return errno;
        #endif

    }

    static inline int socket_get_error(SL_SOCKET fd)
    {
        int socket_error = 0;
        int socket_error_len =  sizeof(int);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *)&socket_error, (socklen_t *)&socket_error_len) < 0)
        {
            return -1;
        }
        return socket_error;
    }

    static inline SL_SOCKET socket_open(int address_family, int type, int protocol)
    {
        return socket(address_family, type, protocol);
    }

    static inline int socket_close(SL_SOCKET fd)
    {
        if ( (fd < 0) || (SL_INVALID_SOCKET == fd) )
        {
            return -1;
        }
        #ifdef SOCKETLITE_OS_WINDOWS
            return closesocket(fd);
        #else
            return close(fd);
        #endif
    }

    static inline int socket_shutdown(SL_SOCKET fd, int how)
    {
        if ( (fd < 0) || (SL_INVALID_SOCKET == fd) )
        {
            return -1;
        }
        return shutdown(fd, how);
    }

    static inline SL_SOCKET socket_accept(SL_SOCKET fd, struct sockaddr *addr, int *addrlen)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            return WSAAccept(fd, addr, addrlen, NULL, 0);
        #else
            return accept(fd, addr, (socklen_t *)addrlen);
        #endif
    }

    static inline int socket_bind(SL_SOCKET fd, const struct sockaddr *addr, int addrlen)
    {
        return bind(fd, addr, addrlen);
    }

    static inline int socket_listen(SL_SOCKET fd, int backlog)
    {
        return listen(fd, backlog);
    }

    static inline int socket_connect(SL_SOCKET fd, const struct sockaddr *addr, int addrlen)
    {
        return connect(fd, addr, addrlen);
    }

    static inline int socket_connect(SL_SOCKET fd, const struct sockaddr *addr, int addrlen, timeval *timeout)
    {
        if (NULL == timeout)
        {
            return connect(fd, addr, addrlen);
        }
        else
        {
            SL_Socket_CommonAPI::socket_set_block(fd, false);
            int ret = connect(fd, addr, addrlen);
            if (ret == 0)
            {
                return 0;
            }
            if (ret == SL_SOCKET_ERROR) 
            {
                fd_set read_set;
                fd_set write_set;
                FD_ZERO(&read_set);
                FD_ZERO(&write_set);
                FD_SET(fd, &read_set);
                FD_SET(fd, &write_set);
                #ifdef SOCKETLITE_OS_WINDOWS
                    int n = select(0, &read_set, &write_set, NULL , timeout);
                #else
                    int n = select(fd+1, &read_set, &write_set, NULL, timeout);
                #endif
                if (n <= 0)
                {
                    return -1;
                }
                if (FD_ISSET(fd, &read_set) || FD_ISSET(fd, &write_set))
                {
                    if (socket_get_error(fd) == 0)
                    {
                        return 0;
                    }
                }
            }
        }
        return -1;
    }

    static inline int socket_set_block(SL_SOCKET fd, bool block)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            {
                unsigned long arg = (block ? 0:1);
                return ioctlsocket(fd, FIONBIO, &arg);
            }
        #else
            //方法1
            int flags = fcntl(fd, F_GETFL);
            if (block)
            {
                flags &= ~O_NONBLOCK;
            }
            else
            {
                flags |= O_NONBLOCK;
            }
            if (fcntl(fd, F_SETFL, flags) == -1) 
            {
                return -1;
            }
            return 0;

            //方法2
            //int arg = (block ? 0:1);
            //return ::ioctl(fd, FIONBIO, &arg);
        #endif
    }

    static inline int socket_set_tcpnodelay(SL_SOCKET fd, int flag)
    {
        return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
    }

    static inline int socket_set_keepalive(SL_SOCKET fd, int flag)
    {
        return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&flag, sizeof(int));
    }

    static inline int socket_set_keepalivevalue(SL_SOCKET fd, ulong keepalivetime = 7200000, ulong keepaliveinterval = 1000)
    {
        #ifdef SOCKETLITE_OS_WINDOWS 
            #if (_WIN32_WINNT>=0x0500)    //window 2000 or later version
                    tcp_keepalive in_keepalive  = {0};
                    tcp_keepalive out_keepalive = {0};
                    ulong inlen  = sizeof(tcp_keepalive);
                    ulong outlen = sizeof(tcp_keepalive);
                    ulong bytesreturn = 0;

                    //设置socket的keepalivetime,keepaliveinterval
                    in_keepalive.onoff = 1;
                    in_keepalive.keepalivetime = keepalivetime;
                    in_keepalive.keepaliveinterval = keepaliveinterval;

                    //为选定的SOCKET设置Keep Alive，成功后SOCKET可通过KeepAlive自动检测连接是否断开 
                    if ( SOCKET_ERROR == WSAIoctl(fd, SIO_KEEPALIVE_VALS,(LPVOID)&in_keepalive, inlen,(LPVOID)&out_keepalive, outlen, &bytesreturn, NULL, NULL) )
                    {
                        return -1;
                    }
            #endif
        #elif defined(SOCKETLITE_OS_LINUX)
                ///* set first test time */ 
                //setsockopt(fd, SOL_TCP, TCP_KEEPIDLE , (const char *)&iIdleTime , sizeof(iIdleTime)); 
                ///* set test idle time */ 
                //setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (const char *)&iInterval, sizeof(iInterval)); 
                ///* set test count */ 
                //setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (const char *)&iCount, sizeof(iCount)); 
        #endif
        return 0;
    }

    static inline int socket_set_reuseaddr(SL_SOCKET fd, int flag)
    {
        return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(int));
    }

    static inline int socket_set_broadcast(SL_SOCKET fd, int flag)
    {
        return setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char *)&flag, sizeof(int));
    }

    static inline int socket_set_send_buffersize(SL_SOCKET fd, int sz)
    {
        return setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *)&sz, sizeof(int));
    }

    static inline int socket_get_send_buffersize(SL_SOCKET fd)
    {
        int sz  = 0;
        int len = sizeof(int);
        if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *)&sz, (socklen_t *)&len) == SL_SOCKET_ERROR)
        {
            return -1;
        }
        return sz;
    }

    static inline int socket_get_recv_buffersize(SL_SOCKET fd)
    {
        int sz  = 0;
        int len = sizeof(int);
        if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&sz, (socklen_t *)&len) == SL_SOCKET_ERROR)
        {
            return -1;
        }
        return sz;
    }

    static inline int socket_set_recv_buffersize(SL_SOCKET fd, int sz)
    {
        return setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&sz, sizeof(int));
    }

    static inline int socket_set_recv_timeout(SL_SOCKET fd, int timeout)
    {
        return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
    }

    static inline int socket_set_send_timeout(SL_SOCKET fd, int timeout)
    {
        return setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(int));
    }

    static inline int socket_set_linger(SL_SOCKET fd, int onoff, int linger_time)
    {
        linger l;
        l.l_onoff  = onoff;
        l.l_linger = linger_time;
        return setsockopt(fd, SOL_SOCKET, SO_LINGER, reinterpret_cast<char *>(&l), sizeof(l));
    }

    static inline int socket_set_dontlinger(SL_SOCKET fd, int onoff)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            return  setsockopt(fd, SOL_SOCKET, SO_DONTLINGER, reinterpret_cast<char *>(&onoff), sizeof(int));
        #else
            return 0;
        #endif
    }

    static inline int socket_recv(SL_SOCKET fd, char *buf, int len, int flags, SL_OVERLAPPED *overlapped, int &error)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            long   bytes_recv = 0;
            WSABUF wsabuf;
            wsabuf.len = len;
            wsabuf.buf = (char *)buf;
            int ret = WSARecv(fd, &wsabuf, 1, (ulong *)&bytes_recv, (LPDWORD)&flags, overlapped, 0);
            if (ret < 0)
            {
                error = socket_get_lasterror_wsa();
                return ret;
            }
            return bytes_recv;
        #else 
            int ret = recv(fd, buf, len, flags);
            if (ret < 0)
            {
                error = socket_get_lasterror_wsa();
            }
            return ret;
        #endif
    }

    static inline int socket_recvn(SL_SOCKET fd, char *buf, int len, int flags, SL_OVERLAPPED *overlapped, int &error)
    {
        long bytes_recv = 0;
        int  ret;
        while (bytes_recv < len )
        {
            ret = recv(fd, buf+bytes_recv, len-bytes_recv, flags);
            if (ret < 0)
            {
                error = socket_get_lasterror_wsa();
                return bytes_recv;
            }
            else
            {
                bytes_recv += ret;
            }
        }
        return bytes_recv;
    }

    static inline int socket_recvv(SL_SOCKET fd, SL_IOVEC *iov, int iovcnt, int flags, SL_OVERLAPPED *overlapped, int &error)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            long bytes_recv = 0;
            int ret = WSARecv(fd, (WSABUF *)iov, iovcnt, (ulong *)&bytes_recv, (LPDWORD)&flags, overlapped, 0);
            if (ret < 0)
            {
                error = socket_get_lasterror_wsa();
                return ret;
            }
            return bytes_recv;
        #else
            int ret = readv(fd, iov, iovcnt);
            if (ret < 0)
            {
                error = socket_get_lasterror_wsa();
            }
            return ret;
        #endif
    }

    static inline int socket_send(SL_SOCKET fd, const char *buf, int len, int flags = 0, SL_OVERLAPPED *overlapped = NULL, int *error = NULL)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            long   bytes_send = 0;
            WSABUF wsabuf;
            wsabuf.len = len;
            wsabuf.buf = (char *)buf;
            int ret = WSASend(fd, &wsabuf, 1, (ulong *)&bytes_send, flags, overlapped, 0);
            if (ret < 0)
            {
                if (NULL != error)
                {
                    *error = socket_get_lasterror_wsa();
                }
                return ret;
            }
            return bytes_send;
        #else 
            int ret = send(fd, buf, len, flags);
            if (ret < 0)
            {
                if (NULL != error)
                {
                    *error = socket_get_lasterror_wsa();
                }
            }
            return ret;
        #endif
    }

    static inline int socket_sendn(SL_SOCKET fd, const char *buf, int len, int flags = 0, SL_OVERLAPPED *overlapped = NULL, int *error = NULL)
    {
        long bytes_send = 0;
        int  ret;
        while (bytes_send < len )
        {
            ret = send(fd, buf+bytes_send, len-bytes_send, flags);
            if (ret < 0)
            {
                if (NULL != error)
                {
                    *error = socket_get_lasterror_wsa();
                }
                return bytes_send;
            }
            else
            {
                bytes_send += ret;
            }
        }
        return bytes_send;
    }

    static inline int socket_sendv(SL_SOCKET fd, const SL_IOVEC *iov, int iovcnt, int flags, SL_OVERLAPPED *overlapped, int &error)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            long  bytes_send = 0;
            int ret = WSASend(fd, (WSABUF *)iov, iovcnt, (ulong *)&bytes_send, flags, overlapped, 0);
            if (ret < 0)
            {
                error = socket_get_lasterror_wsa();
                return ret;
            }
            return bytes_send;
        #else
            int ret = writev(fd, iov, iovcnt);
            if (ret < 0)
            {
                error = socket_get_lasterror_wsa();
            }
            return ret;
        #endif
    }

    static inline int socket_recvfrom(SL_SOCKET fd, char *buf, int len, int flags, struct sockaddr *src_addr, int *addrlen, SL_OVERLAPPED *overlapped, int &error)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            long   bytes_recv = 0;
            WSABUF wsabuf;
            wsabuf.len = len;
            wsabuf.buf = (char *)buf;
            int ret = WSARecvFrom(fd, &wsabuf, 1, (ulong *)&bytes_recv, (LPDWORD)&flags, src_addr, addrlen, overlapped, 0);
            if (ret < 0)
            {
                error = socket_get_lasterror_wsa();
                return ret;
            }
            return bytes_recv;
        #else 
            int ret = recvfrom(fd, buf, len, flags, src_addr, (socklen_t *)addrlen);
            if (ret < 0)
            {
                error = socket_get_lasterror_wsa();
            }
            return ret;
        #endif
    }

    static inline int socket_sendto(SL_SOCKET  fd, const char *buf, int len, int flags, const struct sockaddr *dest_addr, int addrlen, SL_OVERLAPPED *overlapped, int *error)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            long   bytes_send = 0;
            WSABUF wsabuf;
            wsabuf.len = len;
            wsabuf.buf = (char *)buf;
            int ret = WSASendTo(fd, &wsabuf, 1, (ulong *)&bytes_send, flags, dest_addr, addrlen, overlapped, 0);
            if (ret < 0)
            {
                if (NULL != error)
                {
                    *error = socket_get_lasterror_wsa();
                }
                return ret;
            }
            return bytes_send;
        #else 
            int ret = sendto(fd, buf, len, flags, dest_addr, addrlen);
            if (ret < 0)
            {
                if (NULL != error)
                {
                    *error = socket_get_lasterror_wsa();
                }
            }
            return ret;
        #endif
    }

    //sleep以毫秒为单位
    static inline void util_sleep(uint timeout_ms)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            Sleep(timeout_ms);
        #else
            //方法1
            //select();

            //方法2
            //usleep(timeout_ms * 1000);

            //方法3
            struct timespec ts;
            ts.tv_sec  = timeout_ms / 1000;
            ts.tv_nsec = (timeout_ms - ts.tv_sec * 1000) * 1000000L;
            nanosleep(&ts, 0);
        #endif
    }

    //sleep以秒为单位
    static inline void util_sleep_s(uint timeout_s)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            Sleep(timeout_s * 1000);
        #else
            //方法1
            //select函数

            //方法2
            //sleep(timeout_s);

            //方法3
            //usleep(timeout_s * 10000000L);

            //方法4
            struct timespec ts;
            ts.tv_sec  = timeout_s;
            ts.tv_nsec = 0;
            nanosleep(&ts, NULL);
        #endif
    }

    //sleep以毫秒为单位
    static inline void util_sleep_ms(uint timeout_ms)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            Sleep(timeout_ms);
        #else
            //方法1
            //select函数

            //方法2
            //usleep(timeout_ms * 1000);

            //方法3
            struct timespec ts;
            ts.tv_sec  = timeout_ms / 1000;
            ts.tv_nsec = (timeout_ms - ts.tv_sec * 1000) * 1000000L;
            nanosleep(&ts, NULL);
        #endif
    }

    //sleep以微秒为单位
    static inline void util_sleep_us(uint timeout_us)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            Sleep(timeout_us / 1000);
        #else
            //方法1
            //select函数

            //方法2
            //usleep(timeout_us);

            //方法3
            struct timespec ts;
            ts.tv_sec  = timeout_us / 1000000L;
            ts.tv_nsec = (timeout_us - ts.tv_sec * 1000000L) * 1000;
            nanosleep(&ts, NULL);
        #endif
    }

    //sleep以纳秒为单位
    static inline void util_sleep_ns(uint64 timeout_ns)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            Sleep(timeout_ns / 1000000);
        #else
            //方法1
            //select函数

            //方法2
            //usleep(timeout_us);

            //方法3
            struct timespec ts;
            ts.tv_sec  = timeout_ns / 1000000000L;
            ts.tv_nsec = timeout_ns - ts.tv_sec * 1000000000L;
            nanosleep(&ts, NULL);
        #endif
    }

    static inline int64 util_atoi64(const char *str)
    {
        //方法1
        //_atoi64(atoll)

        //方法2
        //_strtoi64(strtoll)

        #ifdef SOCKETLITE_OS_WINDOWS
            return _atoi64(str);
        #else
            return atoll(str);
        #endif
    }

    static inline uint64 util_atoui64(const char *str)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            return _strtoui64(str, NULL, 10);
        #else
            return strtoull(str, NULL, 10);
        #endif
    }

    static inline int util_i64toa(int64 i, char *str, int radix)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            _i64toa(i, str, radix);
        #else
            //lltoa(i, str, radix);
            sprintf(str, "%lld", i);
        #endif
        return 0;
    }

    static inline int util_ui64toa(uint64 i, char *str, int radix)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            _ui64toa(i, str, radix);
        #else
            //ulltoa(i, str, radix);
            sprintf(str, "%llu", i);
        #endif
        return 0;
    }

    static inline int util_get_application_path(SL_TCHAR *path, int path_len)
    {
        int pos = 0;
        #ifdef SOCKETLITE_OS_WINDOWS
            int real_len = GetModuleFileName(NULL, path, path_len);
            for (pos=real_len; pos>0; --pos)
            {
                if (SL_TEXT('\\') == path[pos])
                {
                    break;
                }
            }
        #elif defined(SOCKETLITE_OS_LINUX)
            char proc_path[128] = {0};
            sprintf(proc_path, "/proc/%d/exe", getpid());
            int real_len = readlink(proc_path, path, path_len);
            for (pos=real_len; pos>0; --pos)
            {
                if ('/' == path[pos])
                {
                    break;
                }
            }
        #endif
        if (path_len > pos)
        {
            path[pos+1] = SL_TEXT('\0');
        }
        return pos;
    }

    static inline int util_get_application_name(SL_TCHAR *app_name, int name_len)
    {
        int real_len = 0;
        #ifdef SOCKETLITE_OS_WINDOWS
            real_len = GetModuleFileName(NULL, app_name, name_len);
        #elif defined(SOCKETLITE_OS_LINUX)
            char proc_path[128] = {0};
            sprintf(proc_path, "/proc/%d/exe", getpid());
            real_len = readlink(proc_path, app_name, name_len);
        #endif
        if (name_len > real_len)
        {
            app_name[real_len+1] = SL_TEXT('\0');
        }
        return real_len;
    }

    //取得当前系统时间(自公元1970/1/1 00:00:00以来经过秒数)
    static inline uint64 util_time()
    {
        return time(NULL);
    }

    //取得当前系统时间(自公元1970/1/1 00:00:00以来经过秒数)
    static inline uint64 util_time_s()
    {
        return time(NULL);
    }

    //取得当前系统时间(自公元1970/1/1 00:00:00以来经过毫秒)
    static inline uint64 util_time_ms()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            struct _timeb tb;
            _ftime_s(&tb);
            return (tb.time * 1000LL + tb.millitm);
        #else
            struct timeval tv;
            gettimeofday(&tv, NULL);
            return (tv.tv_sec * 1000LL + tv.tv_usec / 1000LL);
        #endif
    }

    //取得当前系统时间(自公元1970/1/1 00:00:00以来经过微秒)
    static inline uint64 util_time_us()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            struct _timeb tb;
            _ftime_s(&tb);
            return (tb.time * 1000000LL + tb.millitm * 1000LL);
        #else
            struct timeval tv;
            gettimeofday(&tv, NULL);
            return (tv.tv_sec * 1000000LL + tv.tv_usec);
        #endif
    }

    //取得当前系统时间(自公元1970/1/1 00:00:00以来经过纳秒)
    static inline uint64 util_time_ns()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            struct _timeb tb;
            _ftime_s(&tb);
            return (tb.time * 1000000000LL + tb.millitm * 1000000LL);
        #else
            //方法1
            struct timespec tp;
            clock_gettime(CLOCK_REALTIME, &tp);
            return (tp.tv_sec * 1000000000LL + tp.tv_nsec);

            ////方法2
            //struct timeval tv;
            //gettimeofday(&tv, NULL);
            //return (tv.tv_sec * 1000000000LL + tv.tv_usec * 1000LL);
        #endif
    }

    //取得当前系统时间(自公元1970/1/1 00:00:00以来经过秒数,可以精确到微秒)
    static inline int util_gettimeofday(struct timeval *tv, struct timezone *tz)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            struct _timeb tb;
            _ftime_s(&tb);

            tv->tv_sec  = tb.time;
            tv->tv_usec = tb.millitm * 1000L;
        #else
            gettimeofday(tv, tz);
        #endif
        return 0;
    }

    //时间操作函数
    static inline void util_timeradd(struct timeval *tvp, struct timeval *uvp, struct timeval *vvp)
    {
        vvp->tv_sec  = tvp->tv_sec + uvp->tv_sec;
        vvp->tv_usec = tvp->tv_usec + uvp->tv_usec;
        if (vvp->tv_usec >= 1000000)
        {
            ++vvp->tv_sec;
            vvp->tv_usec -= 1000000;
        }
    }

    static inline void util_timersub(struct timeval *tvp, struct timeval *uvp, struct timeval *vvp)
    {
        vvp->tv_sec  = tvp->tv_sec - uvp->tv_sec;
        vvp->tv_usec = tvp->tv_usec - uvp->tv_usec;
        if (vvp->tv_usec < 0)
        {
            --vvp->tv_sec;
            vvp->tv_usec += 1000000;
        }
    }

    static inline bool util_timerisset(struct timeval *tvp)
    {
        return (tvp->tv_sec || tvp->tv_usec);
    }

    static inline void util_timerclear(struct timeval *tvp)
    {
        tvp->tv_sec = tvp->tv_usec = 0;
    }

    //时间比较函数
    //The return value for each of these functions indicates the lexicographic relation of tvp to uvp
    //<0 tvp less than uvp
    //=0 tvp identical to uvp
    //>0 tvp greater than uvp
    static inline int util_timercmp(struct timeval *tvp, struct timeval *uvp)
    {
        if (tvp->tv_sec > uvp->tv_sec)
        {
            return 1;
        }
        else
        {
            if (tvp->tv_sec < uvp->tv_sec)
            {
                return -1;
            }
            if (tvp->tv_usec > uvp->tv_usec)
            {
                return 1;
            }
            else if (tvp->tv_usec < uvp->tv_usec)
            {
                return -1;
            }
        }
        return 0;
    }

    //取得进程时钟滴答(不受修改系统时钟影响/调整系统时间无关 毫秒:ms 只能用于计时)
    static inline uint64 util_gettickcount()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            //方法1 精度低些, 精确到 10-35ms
            //return GetTickCount();

            //方法2 精度高些, 精确到 1ms
            return timeGetTime();
        #else  
            //方法1
            struct timespec tp;
            clock_gettime(CLOCK_MONOTONIC, &tp);
            return (tp.tv_sec * 1000LL + tp.tv_nsec / 1000000LL);

            //方法2
            //struct timeval tv;
            //gettimeofday(&tv, NULL);
            //return (tv.tv_sec*1000L + tv.tv_usec/1000);
        #endif
    }

    //取得进程时钟滴答(不受修改系统时钟影响/调整系统时间无关 秒:s 只能用于计时)
    static inline uint64 util_process_clock_s()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            //方法1 精度低些, 精确到 10-35ms
            //return GetTickCount();

            //方法2 精度高些, 精确到 1ms
            return (timeGetTime() / 1000LL);
        #else
            //方法1
            struct timespec tp;
            clock_gettime(CLOCK_MONOTONIC, &tp);
            return tp.tv_sec;

            //方法2
            //struct timeval tv;
            //gettimeofday(&tv, NULL);
            //return tv.tv_sec;
        #endif
    }

    //取得进程时钟滴答(不受修改系统时钟影响/调整系统时间无关 毫秒:ms 只能用于计时)
    static inline uint64 util_process_clock_ms()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            //方法1 精度低些, 精确到 10-35ms
            //return GetTickCount();

            //方法2 精度高些, 精确到 1ms
            return timeGetTime();
        #else
            //方法1
            struct timespec tp;
            clock_gettime(CLOCK_MONOTONIC, &tp);
            return (tp.tv_sec * 1000LL + tp.tv_nsec / 1000000LL);

            //方法2
            //struct timeval tv;
            //gettimeofday(&tv, NULL);
            //return (tv.tv_sec * 1000L + tv.tv_usec / 1000);
        #endif
    }

    //取得进程时钟滴答(不受修改系统时钟影响/调整系统时间无关 微秒:us 只能用于计时)
    static inline uint64 util_process_clock_us()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            //方法1 精度低些, 精确到 10-35ms
            //return (GetTickCount() * 1000);

            //方法2 精度高些, 精确到 1ms
            return (timeGetTime() * 1000LL);
        #else
            //方法1
            struct timespec tp;
            clock_gettime(CLOCK_MONOTONIC, &tp);
            return (tp.tv_sec * 1000000LL + tp.tv_nsec / 1000LL);

            //方法2
            //struct timeval tv;
            //gettimeofday(&tv, NULL);
            //return (tv.tv_sec * 1000000LL + tv.tv_usec);
        #endif
    }

    //取得进程时钟滴答(不受修改系统时钟影响/调整系统时间无关 纳秒:ns 只能用于计时)
    static inline uint64 util_process_clock_ns()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            //方法1 精度低些, 精确到 10-35ms
            //return (GetTickCount() * 1000);

            //方法2 精度高些, 精确到 1ms
            return (timeGetTime() * 1000000LL);
        #else
            //方法1
            struct timespec tp;
            clock_gettime(CLOCK_MONOTONIC, &tp);
            return (tp.tv_sec * 1000000000LL + tp.tv_nsec);

            //方法2
            //struct timeval tv;
            //gettimeofday(&tv, NULL);
            //return (tv.tv_sec * 1000000000LL + tv.tv_usec * 1000L);
        #endif
    }

    //两个进程时钟差
    static inline long util_process_clock_difference(ulong end_clock, ulong start_clock)
    {
        return (end_clock - start_clock);
    }

    //取得当前时间戳(不受修改系统时钟影响/调整系统时间无关 毫秒:ms 只能用于计时)
    static inline uint64 util_timestamp()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            //方法1 精度低些, 精确到 10-35ms
            //return GetTickCount();

            //方法2 精度高些, 精确到 1ms
            return timeGetTime();
        #else
            //方法1
            struct timespec tp;
            clock_gettime(CLOCK_MONOTONIC, &tp);
            return (tp.tv_sec * 1000LL + tp.tv_nsec / 1000000LL);

            //方法2
            //struct timeval tv;
            //gettimeofday(&tv, NULL);
            //return (tv.tv_sec * 1000L + tv.tv_usec / 1000);
        #endif
    }

    //取得当前时间戳(不受修改系统时钟影响/调整系统时间无关 毫秒:s 只能用于计时)
    static inline uint64 util_timestamp_s()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            //方法1 精度低些, 精确到 10-35ms
            //return GetTickCount();

            //方法2 精度高些, 精确到 1ms
            return (timeGetTime() / 1000LL);
        #else
            //方法1
            struct timespec tp;
            clock_gettime(CLOCK_MONOTONIC, &tp);
            return tp.tv_sec;

            //方法2
            //struct timeval tv;
            //gettimeofday(&tv, NULL);
            //return tv.tv_sec;
        #endif
    }

    //取得当前时间戳(不受修改系统时钟影响/调整系统时间无关 毫秒:ms 只能用于计时)
    static inline uint64 util_timestamp_ms()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            //方法1 精度低些, 精确到 10-35ms
            //return GetTickCount();

            //方法2 精度高些, 精确到 1ms
            return timeGetTime();
        #else
            //方法1
            struct timespec tp;
            clock_gettime(CLOCK_MONOTONIC, &tp);
            return (tp.tv_sec * 1000LL + tp.tv_nsec / 1000000LL);

            //方法2
            //struct timeval tv;
            //gettimeofday(&tv, NULL);
            //return (tv.tv_sec * 1000L + tv.tv_usec / 1000);
        #endif
    }

    //取得当前时间戳(不受修改系统时钟影响/调整系统时间无关 微秒:us 只能用于计时)
    static inline uint64 util_timestamp_us()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            //方法1 精度低些, 精确到 10-35ms
            //return (GetTickCount() * 1000);

            //方法2 精度高些, 精确到 1ms
            return (timeGetTime() * 1000LL);
        #else
            //方法1
            struct timespec tp;
            clock_gettime(CLOCK_MONOTONIC, &tp);
            return (tp.tv_sec * 1000000LL + tp.tv_nsec / 1000LL);

            //方法2
            //struct timeval tv;
            //gettimeofday(&tv, NULL);
            //return (tv.tv_sec * 1000000LL + tv.tv_usec);
        #endif
    }

    //取得当前时间戳(不受修改系统时钟影响/调整系统时间无关 纳秒:ns 只能用于计时)
    static inline uint64 util_timestamp_ns()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            //方法1 精度低些, 精确到 10-35ms
            //return (GetTickCount() * 1000);

            //方法2 精度高些, 精确到 1ms
            return (timeGetTime() * 1000000LL);
        #else
            //方法1
            struct timespec tp;
            clock_gettime(CLOCK_MONOTONIC, &tp);
            return (tp.tv_sec * 1000000000LL + tp.tv_nsec);

            //方法2
            //struct timeval tv;
            //gettimeofday(&tv, NULL);
            //return (tv.tv_sec * 1000000000LL + tv.tv_usec * 1000L);
        #endif
    }

    //取得系统高精度计数器频率(用于windows系统高精度计时)
    static inline uint64 util_system_counter_frequency()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            static uint64 counter_frequency = 1;
            if (1 == counter_frequency)
            {
                LARGE_INTEGER i;
                if (QueryPerformanceFrequency(&i))
                {
                    counter_frequency = i.QuadPart;
                }
            }
            return counter_frequency;
        #else
            return 1;
        #endif
    }

    //取得系统高精度计数器的计数值
    static inline uint64 util_system_counter()
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            LARGE_INTEGER i;
            QueryPerformanceCounter(&i);
            return i.QuadPart;
        #else
            struct timespec tp;
            clock_gettime(CLOCK_MONOTONIC, &tp);
            return (tp.tv_sec * 1000000000LL + tp.tv_nsec);
        #endif
    }

    //计算系统高精度计数器的计时(两次计数值之差即时间差: 以微秒为时间单位)
    static inline uint64 util_system_counter_time_us(uint64 counter_end, uint64 counter_start)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            return (counter_end - counter_start) * 1000000LL / system_counter_frequency_;
        #else
            return (counter_end - counter_start) / 1000;
        #endif
    }

    //计算系统高精度计数器的计时(两次计数值之差即时间差: 以纳秒为时间单位)
    static inline uint64 util_system_counter_time_ns(uint64 counter_end, uint64 counter_start)
    {
        #ifdef SOCKETLITE_OS_WINDOWS
            return (counter_end - counter_start) * 1000000000LL / system_counter_frequency_;
        #else
            return counter_end - counter_start;
        #endif
    }

    //内存拷贝
    static inline void util_memcpy(void *dest, const void *src, size_t n)
    {
        sl_memcpy(dest, src, n);
    }

    //内存拷贝(这种方式实现, 性能比较差)
    static inline void util_memcpy_char(void *dest, const void *src, size_t n)
    {
        char *dest_char = (char *)dest;
        char *src_char  = (char *)src;
        while (n--) 
        {
            *dest_char++ = *src_char++;
        }
    }

    //检测文件是否存在
    static inline bool util_is_exists_file(const SL_TCHAR *path_name)
    {
        return SL_OS_API::futil_exists(path_name);
    }

    //取得文件大小
    static inline int util_get_file_size(const SL_TCHAR *file_name, unsigned long &file_size)
    {
        return SL_OS_API::futil_filesize(file_name, file_size);
    }

    //取得错误号
    static inline int util_get_errno()
    {
        return errno;
    }

    //取得本地系统所采用的字节序
    static inline int8 util_get_byteorder()
    {
        static int8 byte_order = 0;
        if (byte_order < 1)
        {
            const int e = 1;
            if (*(char *)&e)
            {  
                byte_order = __SOCKETLITE_LITTLE_ENDIAN;
            }
            else
            {
                byte_order = __SOCKETLITE_BIG_ENDIAN;
            }
        }
        return byte_order;
    }

    //64位整数的主机字节序转为网络字节序
    static inline int64 util_htonll(int64 host_int64)
    {
        #if SOCKETLITE_BYTE_ORDER == __SOCKETLITE_BIG_ENDIAN
            return host_int64;
        #else
            #ifdef SOCKETLITE_OS_WINDOWS
                return ( (((unsigned long long)htonl(host_int64)) << 32) + htonl(host_int64 >> 32) );
            #else
                return bswap_64(host_int64);
            #endif
        #endif
    }

    //64位整数的网络字节序转为主机字节序
    static inline int64 util_ntohll(int64 net_int64)
    {
        #if SOCKETLITE_BYTE_ORDER == __SOCKETLITE_BIG_ENDIAN
            return net_int64;
        #else
            #ifdef SOCKETLITE_OS_WINDOWS
                return ( (((unsigned long long)ntohl(net_int64)) << 32) + ntohl(net_int64 >> 32) );
            #else
                return bswap_64(net_int64);
            #endif
        #endif
    }

private:
    static uint     timer_resolution_ms_;           //定时精度(毫秒) windows下1-5 ms
    static uint64   system_counter_frequency_;      //系统高精度计数器的频率
};

#endif

