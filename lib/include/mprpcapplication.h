#pragma once

#include"mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"

class MprpcApplication
{
private:

    static MprpcConfig m_config;
    MprpcApplication(){}//构造函数

    MprpcApplication(const MprpcApplication&) =delete;//拷贝构造函数
    MprpcApplication(MprpcApplication&&) = delete;//移动构造函数
    
public:
    static MprpcConfig& GetConfig();
    static void Init(int argc, char**argv); //加载和解析配置文件
    static MprpcApplication& GetInstance();//单例方法
};


