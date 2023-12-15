#ifndef INETADDRESS_H
#define INETADDRESS_H

#include <netinet/in.h>
#include <string>
// 封装socket地址类
class InetAddress
{
public:
    InetAddress() = default;
    explicit InetAddress(std::string ip, uint16_t port);
    explicit InetAddress(const sockaddr_in& addr);
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;
    const sockaddr_in* getSockAddr() const;

    void setSockAddr(const sockaddr_in &addr) { addr_ = addr; }
private:
    sockaddr_in addr_;
};



#endif