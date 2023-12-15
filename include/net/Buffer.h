#ifndef BUFFER_H
#define BUFFER_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>



class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;


    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize)
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend)
    {}

    ~Buffer() = default;

    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const { return readerIndex_; }

    // 把[data, data + len]内存上的数据，添加到writeable缓存区当中
    void append(const char *data, size_t len)
    {
        ensureWriteableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }

    char* beginWrite()
    {
        return begin() + writerIndex_;
    }

    const char* beginWrite() const
    {
        return begin() + writerIndex_;
    }

    char* beginRead()
    {
        return begin() + readerIndex_;
    }

    const char* beginRead() const 
    {
        return begin() + readerIndex_;
    }

    const char* peek() const
    {
        return begin() + readerIndex_;
    }

    void retrive(size_t len)
    {
        if(len < readableBytes())
        {
            readerIndex_ += len;
        }
        else
        {
            retriveAll();
        }
    }

    void retriveAll()
    {
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    // 把onMessage函数上报的buffer的数据，转换成string类型的数据返回
    
    std::string retrieveAllAsString()
    {
        return retriveAsString(readableBytes());
    }

    std::string retriveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrive(len); // 对缓存区进行复位操作
        return result;
    }


    void ensureWriteableBytes(size_t len)
    {
        if(writableBytes() < len)
        {
            makeSpace(len);
        }
        else
        {}
    }
    ssize_t readFd(int fd, int *saveErrno);

    ssize_t writeFd(int fd, int *saveErrno);
private:
    char* begin()
    {
        return &*buffer_.begin();
    }

    const char* begin() const
    {
        return &*buffer_.begin();
    }


     
    // 扩容函数
    void makeSpace(size_t len)
    {
        if(writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            size_t readable = readableBytes();
            // 把readcache向前挪
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};



#endif