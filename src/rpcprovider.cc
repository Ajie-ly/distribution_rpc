#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"
/*
service_name ->service描述
                        ->service* 记录服务对象
                          method_name method方法对象

json        文本储存，键值对
protobuf    二进制存储，仅携带数据


*/
// 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;

    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserverDesc = service->GetDescriptor();
    // 获取服务的名字
    std::string service_name = pserverDesc->name();
    // 获取服务对象service的方法的数量
    int methodCnt = pserverDesc->method_count();

    // std::cout << "service_name: " << service_name <<std::endl;
    LOG_INFO(service_name.c_str());
    for (int i = 0; i < methodCnt; ++i)
    {
        // 获取了服务对象指定下标的服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor *pmethodDesc = pserverDesc->method(i);
        std::string method_name = pmethodDesc->name();
        // 存储在方法的映射中
        service_info.m_methodMap.insert({method_name, pmethodDesc});

        // std::cout << "method_name: " << method_name <<std::endl;
        LOG_INFO(method_name.c_str());
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}
// 启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run()
{

    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);
    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");
    // 绑定连接回调和消息读写回调方法,分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // 设置muduo的线程数量
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册刀zk上面，让rpc client可以从zk上找到这个服务
    ZkClient zkCli;
    zkCli.Start();
    // service_name 为永久性节点，   method_name为临时性节点
    for (auto &sp : m_serviceMap)
    {
        // /service_name
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_methodMap)
        {
            // /service_name/method_name
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    // rpc服务端准备启动，打印信息
    std::cout << "RpcProvider start service at ip: " << ip << " port: " << port << std::endl;

    // 启动服务
    server.start();
    m_eventLoop.loop();
}

// 新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 和rpc client的连接断开了
        conn->shutdown();
    }
}

/*
在框架内部，RpcProvider和RpcConsumer协商好，之间通信用的protobuf数据类型
service_name(通过name获得服务) method_name(获得方法) args   定义proto的message类型，进行数据头的序列化和反序列化
                                                          service_name method_name args_size
16UserServiceLoginzhang san12356(无法分辨谁是服务，方法和参数)

header_size(4个字节) + header_str + args_str
header_size用二进制存储， std::string insert和copy方法
10 "10"
10000 "100000"
*/
// 已建立连接用户的读写事件回调，如果一远程有个rpc服务的调用请求，那么OnMessage方法就会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buffer,
                            muduo::Timestamp)
{
    // 网络上接受的远程rpc调用请求的字节流  Login args
    std::string recv_buf = buffer->retrieveAllAsString();

    // 从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0);

    // 根据header_size读取数据头的原视字符流，反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size); // 根据前面的大小，获得header_str二进制字符流
    mprpc::RpcHeader rpc_Header;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpc_Header.ParseFromString(rpc_header_str)) // 将二进制转化
    {
        // 数据头序列化成功,将对应的值取出来
        service_name = rpc_Header.service_name();
        method_name = rpc_Header.method_name();
        args_size = rpc_Header.args_size();
    }
    else
    {
        // 数据头序列化失败
        std::cout << "rpc_header_str: " << rpc_header_str << "parse error!" << std::endl;
        return;
    }

    // 获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    std::cout << "-----------------------------------------------" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "-----------------------------------------------" << std::endl;

    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        // 说明没有service
        std::cout << service_name << "is not exist!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ":" << method_name << "is not exist!" << std::endl;
        return;
    }

    // 获取service对象 UserService
    google::protobuf::Service *service = it->second.m_service;
    // 获取method对象 Login
    const google::protobuf::MethodDescriptor *method = mit->second;

    // 生成rpc方法调用的请求request和响应respone参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        std::cout << "request parse error, content: " << args_str << std::endl;
    }

    google::protobuf::Message *respone = service->GetResponsePrototype(method).New();

    // 给下面的method方法调用，绑定一个Closure的回调函数
    google::protobuf::Closure *done =
        google::protobuf::NewCallback<RpcProvider,
                                      const muduo::net::TcpConnectionPtr &,
                                      google::protobuf::Message *>(this, &RpcProvider::SendRpcResponse, conn, respone);

    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    // new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, respone, done);
}
// Closure的回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializePartialToString(&response_str))
    {
        // 序列化成功后，通过网络把rpc方法执行的结果发送回rpc调用方
        conn->send(response_str);
    }
    else
    {
        std::cout << "serialize response_str error!" << std::endl;
    }
    conn->shutdown();
}
