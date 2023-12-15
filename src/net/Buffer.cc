#include "Buffer.h"
#include "Logger.h"
#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>



ssize_t Buffer::readFd(int fd, int *saveErrno)
{
    char extrabuf[65535] = {0};

    struct iovec vec[2];

    const size_t writeable = writableBytes();
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writeable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writeable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    
    LOG_DEBUG("%s:%s:%d 正在读取数据 内容 : %s \n", __FILE__, __FUNCTION__, __LINE__, (char*)vec[0].iov_base);
    
    if (n < 0)
    {
        *saveErrno = errno;
    }
    else if (n <= writeable)
    {
        writerIndex_ += n;
    }
    else // extrabuf也写入了数据
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writeable);
    }

    return n;
}

ssize_t Buffer::writeFd(int fd, int *saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if(n < 0)
    {
        *saveErrno = errno;
    }

    return n;
}
