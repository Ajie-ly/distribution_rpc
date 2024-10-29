#include"test.pb.h"
#include<iostream>
#include<string>
using namespace fixbug;//工作中不要这样！！防止命名空间污染

int main()
{
    // LoginResponse rsp;
    // ResultCode * rc =rsp.mutable_result();
    // rc->set_errcode(1);
    // rc->set_errmsg("登陆处理失败了");

    GetFriendListResponse rsp;
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(0);
    
    User *user1 = rsp.add_friend_list();
    user1->set_name("ajie1");
    user1->set_age(20);
    user1->set_sex(User::MAN);

    User *user2 = rsp.add_friend_list();
    user2->set_name("ajie2");
    user2->set_age(22);
    user2->set_sex(User::MAN);

    std::cout << rsp.friend_list_size() << std::endl;

    return 0;

}




int main1()
{
    //封装login请求对象的数据
    LoginRequest req;
    req.set_name("ajie");
    req.set_pwd("123456");

    std::string send_str;
    //将成员序列化为字符串
    if(req.SerializeToString(&send_str))
    {
        std::cout << send_str.c_str() << std::endl;
    }
    //从send_str反序列化一个Login请求对象
    LoginRequest reqB;
    if (reqB.ParseFromString(send_str))
    {
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }
    return 0;
}