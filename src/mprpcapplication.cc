#include "mprpcapplication.h"
#include <iostream>
#include<string>
#include<unistd.h>

MprpcConfig MprpcApplication::m_config;

void ShowArgsHelp()
{
    std::cout << "format: conmand -i <configfile>" << std::endl;
}

void MprpcApplication::Init(int argc, char**argv)
{
    if(argc < 2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    std::string config_file; // config_file存储-i后面的文件名
    while((c = getopt(argc, argv,"i:")) != -1) //"i:"表示-i
    {
        switch (c)
        {
        case 'i': //如果是-i
            config_file = optarg;
            break;
        case '?'://如果是未知命令
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':// -i 后面没有参数
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
        //开始加载配置文件 rpcserver_ip= rcpserver_port= zookepper_port=
        m_config.LoadConfigFile(config_file.c_str());
    //     std::cout << "rpcserverip:" << m_config.Load("rpcserverip") << std::endl;
    //     std::cout << "rpcserverport:" << m_config.Load("rpcserverport") << std::endl;
    //     std::cout << "zookeeperip:" << m_config.Load("zookeeperip") << std::endl;
    //     std::cout << "zookeeperport:" << m_config.Load("zookeeperport") << std::endl;
    // 
    }


}
MprpcApplication& MprpcApplication::GetInstance()//单例方法
{
    static MprpcApplication app;
    return app;
}

MprpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}