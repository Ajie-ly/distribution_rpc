#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

// UserService是一个本地服务，提供了两个进程内的本地方法Login和GetFriendLists
//先写本地服务，重写UserServiceRpc里面的相应函数，里面：框架得到从muduo库传过来的信息，然后调用本地服务，产生结果返回值
class UserService :public fixbug::UserServiceRpc
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service:Login" << std::endl;
        std::cout << "name: " << name << "pwd:" << pwd << std::endl;
        return true;
    }
     bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service:Register" << std::endl;
        std::cout << "id:" << id << "name: " << name << "pwd:" << pwd << std::endl;
        return true;
    }


    //这是站在使用角度分析rpc的功能
    //重写基类UserServiceRpc函数
    //1.caller(client) ==> Login(LoginRequest) => muduo => callee(server)
    //2.callee(server) ==> Login(LoginRequest) => 转发到重写的Login方法上
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)

    {
        //1.框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();
        
        //2.做本地业务
        bool login_result = Login(name,pwd);

        //3.把响应写入（错误码，错误消息，返回值）
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        //4.做回调操作，执行响应对象数据的序列化和网络发送（由框架完成）
        done->Run();
    }

   void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterReponse* response,
                       ::google::protobuf::Closure* done)
    {
        //从框架里面得到调用本地服务所需要的信息
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();
        //调用本地服务
        bool ret = Register(id,name,pwd);
        
        //返回的内容在proto里面已经定义好
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);

        done->Run();
    }



};

int main(int argc, char** argv)
{
    //调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    //provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    //启动一个rpc服务发布节点，Run之后，进程进入阻塞状态，等待远程rpc请求
    provider.Run();
}