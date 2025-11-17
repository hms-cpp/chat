#pragma once

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
#include <string>
#include <iostream>

#include "config.hpp"
#include"public.hpp"

using std::cerr;
using std::endl;
using std::cout;
using std::function;
using std::string;
using std::thread;

class Redis
{
public:
    Redis();
    ~Redis();

    bool connect();

    // 发布消息
    bool publish(int channel, string message);

    // 向指定通道订阅消息
    bool subscribe(int channel);

    // 取消订阅
    bool unsubscribe(int channel);

    // 在独立线程中接受订阅通道的消息
    void observer_channel_message();

    // 初始化向业务层上报通道消息的回调对象
    void init_notify_handler(function<void(int, string)> fn);

private:
    // hiredis同步上下文对象，负责publish消息
    redisContext *_publish_context;

    // 负责subscribe消息
    redisContext *_subscribe_context;

    // 回调操作
    function<void(int, string)> _notify_message_handler;
};