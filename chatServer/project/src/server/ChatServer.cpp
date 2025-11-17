#include "ChatServer.hpp"
#include "json.hpp"
#include "ChatService.hpp"
#include "public.hpp"

#include <muduo/base/Logging.h>
#include <functional>
#include <string>


using json = nlohmann::json;

ChatServer::ChatServer(muduo::net::EventLoop *loop_,
                       const muduo::net::InetAddress &listenAddr_, const std::string &nameArg_)
    : _server(loop_, listenAddr_, nameArg_),
      _loop(loop_)
{
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    _server.setThreadNum(16);
}

void ChatServer::start()
{
    ChatService::instance()->reset();
    _server.start();
}

void ChatServer::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        LOG_INFO << "New connection established: " << conn->peerAddress().toIpPort() 
                 << " -> " << conn->localAddress().toIpPort();
    }
     else
    {
        LOG_INFO << "Connection closed: " << conn->name() 
                 << " (" << conn->peerAddress().toIpPort() << ")";
        
        ChatService::instance()->clientCloseException(conn);
    }
}
void ChatServer::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp time)
{
    std::string buf = buffer->retrieveAllAsString();
    debug("receive:");
    debug(buf);
    json js = json::parse(buf);

    auto msgHandler = ChatService::instance()->getHandler(js["msg_id"].get<int>());
    msgHandler(conn, js, time);
}