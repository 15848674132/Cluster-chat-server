#include "InetAddress.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

InetAddress::InetAddress(std::string ip, uint16_t port)
{
    memset(&addr_, sizeof addr_, 0);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

InetAddress::InetAddress(const sockaddr_in &addr)
    : addr_(addr)
{
}

std::string InetAddress::toIp() const
{
    char buf[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    return buf;
}

std::string InetAddress::toIpPort() const
{
    char buf[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t end = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf + end, ":%u", port);
    return buf;

}

uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port);
}

const sockaddr_in *InetAddress::getSockAddr() const
{
    return &addr_;
}



// #include <iostream>

//  int main()
//  {
//     InetAddress addr(8080, "127.0.0.1");
//     std::cout <<addr.toIp() << std::endl;
//     std::cout <<addr.toPort() << std::endl;
//     std::cout <<addr.toIpPort() << std::endl;
//     return 0;
//  }