#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include<iostream>
#include "logger.h"

class FriendService:public fixbug::FriendServiceRpc
{
    public:
    std::vector<std::string> GetFriendsLists(uint32_t id)
    {
        std::cout << "do GetFriendsList service, userid: " <<id << std::endl;
        std::vector<std::string> vec;
        vec.push_back("gao yang");
        vec.push_back("zhang da");
        vec.push_back("wang er");
        vec.push_back("li san");
        return vec;
    }

    void GetFriendLists(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendListsRequest* request,
                       ::fixbug::GetFriendListsResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t userid = request->userid();
        std::vector<std::string> friendsList = GetFriendsLists(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for(std::string &name : friendsList)
        {
            std::string *p = response->add_friends();
            *p = name;
        }
        done->Run();

    }
};

int main(int argc, char** argv)
{   
    LOG_INFO("first log message!");
    LOG_ERR("%s:%s:%d",__FILE__,__FUNCTION__,__LINE__);

    //调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    //provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    //启动一个rpc服务发布节点，Run之后，进程进入阻塞状态，等待远程rpc请求
    provider.Run();
}