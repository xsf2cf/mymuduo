#include "Buffer.h"
//#include "Logger.h"

#include<errno.h>
#include<sys/uio.h>
#include<unistd.h>

ssize_t Buffer::readFd(int fd, int* saveErrno)
{
    char extrabuf[65536] = {0};

    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writableBytes() < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if(n < 0)
    {
        *saveErrno = errno;
    }
    else if( n <= writable)
    {
        writerIndex_ += n;
    }
    else{
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}

ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    //LOG_DEBUG("可写字数%d, 文件描述符%d", readableBytes(), fd);
    if(n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}