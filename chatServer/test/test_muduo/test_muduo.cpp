#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>

class ChatServer
{
public:
    ChatServer(muduo::net::EventLoop *loop_,
               const muduo::net::InetAddress &listenAddr_,
               const std::string &nameArg_)
        : _server(loop_, listenAddr_, nameArg_),
          _loop(loop_)
    {
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,std::placeholders::_1));
        _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
        _server.setThreadNum(12);
    }

    ~ChatServer()
    {

    }


    void start()
    {
        _server.start();
    }

private:
    void onConnection(const muduo::net::TcpConnectionPtr &conn)
    {
        if(conn->connected())
        {
            std::cout<<conn->peerAddress().toIpPort()<<"->"<<
            conn->localAddress().toIpPort()<<
            "state: online "<<std::endl;
        }
        else
        {
            std::cout<<conn->peerAddress().toIpPort()<<"->"<<
            conn->localAddress().toIpPort()<<
            "state: offline "<<std::endl;
            conn->shutdown();
            //_loop->quit();
        }
    }
    void onMessage(const muduo::net::TcpConnectionPtr&conn,muduo::net::Buffer *buffer,muduo::Timestamp time)
    {
        std::string buf=buffer->retrieveAllAsString();
        std::cout<<"receive data: "<<buf<<"time: "<<time.toString()<<std::endl;
        conn->send(buf);
    }

    muduo::net::TcpServer _server;
    muduo::net::EventLoop *_loop;
};

int main()
{
    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr("127.0.0.1",8900);
    ChatServer server(&loop,addr,"ChatServer");
    
    server.start();
    loop.loop();

    return 0;
}