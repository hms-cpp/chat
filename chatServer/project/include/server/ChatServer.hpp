#pragma once

#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>

class ChatServer
{
    public:

    ChatServer(muduo::net::EventLoop *loop_,
        const muduo::net::InetAddress &listenAddr_,const std::string &nameArg_);

    void start();

    private:
    //连接相关的回调
    void onConnection(const muduo::net::TcpConnectionPtr&);

    void onMessage(const muduo::net::TcpConnectionPtr&,muduo::net::Buffer *,muduo::Timestamp);


    muduo::net::TcpServer _server;
    muduo::net::EventLoop *_loop;

};