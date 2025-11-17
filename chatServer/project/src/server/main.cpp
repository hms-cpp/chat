#include"ChatServer.hpp"
#include"UserModel.hpp"
#include"ChatService.hpp"
#include"config.hpp"

#include<signal.h>
#include<iostream>

void resetHander(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc,char**argv)
{
    char *ip = IP;
    uint16_t port = PORT;
    if (argc < 2)
    {
    }
    else
    {
        ip = argv[1];
        port = atoi(argv[2]);
    }
    signal(SIGINT,resetHander);

    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr(ip,port);
    ChatServer server(&loop,addr,"chat");

    server.start();
    loop.loop();

    return 0;
}
