#pragma once
#include "google/protobuf/service.h"
#include <memory>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <string>
#include <unordered_map>



class RpcProvider
{
public:
    // 这里参数是Service的基类，proto中定义的所有service都是其派生类
    // 框架提供给用户使用，可以发布rpc方法的接口函数
    void NotifyService(google::protobuf::Service *service);

    // 启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();

private:
    // 组合了EventLoop
    muduo::net::EventLoop m_eventLoop;
    // 新的socket连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr &);
    // 已建立连接用户的读写事件回调 ，
    void OnMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp);
    //service服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service; //保存服务对象
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor*> m_methodMap;//保存服务方法
    };
    //存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;
    //Closure的回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&,google::protobuf::Message*);
    
};