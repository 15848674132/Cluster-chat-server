#include "chatServer.hpp"
#include "chatService.hpp"
#include <iostream>
#include <signal.h>

using namespace std;
void resetHandler(int sigNum)
{
    ChatService::getInstance()->reset();
    exit(0);
}

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        return 0;
    }
    
    signal(SIGINT, resetHandler);
    EventLoop mainLoop;
    InetAddress listenAddr(argv[1], atoi(argv[2]));
    ChatServer server(&mainLoop, listenAddr, "ChatServer");
    server.start();
    mainLoop.loop();
    
    return 0;
}