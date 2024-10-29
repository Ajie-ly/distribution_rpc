## 总结
本框架主要有三个模块，rpc服务调用方，rpc服务提供方，ZooKeeper

1.rpc服务提供方与ZooKeeper建立一个session对话<br>
2.rpc服务提供方会维护一个rpc方法的map表<br>
3.使用muduo库绑定回调在OnConnection和OnMessage方法上，接受网络RPC的调用，muduo库启动网络服务.我们创建了1个I/O线程，3个worker线程.，开启start和loop，然后Rpc服务节点启动<br> 
4.通过ZkClient在ZooKeeper上注册节点 
```
/UserServiceRpc 永久性节点
    /login ip:port 临时性节点
    /reg   ip:port 
``` 
5.rpc调用方通过Stub代理对象来调用rpc方法，并通过CallMethod方法来打包service_name、method_name和对数据的序列化。然后在ZkCli客户端上获取到service_name和method_name对应的ip和port <br>
6.rpc服务调用方对package进行封装，来避免TCP连接的粘包问题. <br>
7.打包好数据以后，就可以直接跟rpc服务提供方进行调用请求。rpc服务提供方响应后，给与响应，并关闭connection连接
<br>
8.服务提供方的操作:
- 服务提供方使用muduo的onMessage获取网络字节流和反序列化参数.
- 生成LoginRequest和LoginResponse.
- 根据service_name和method_name找到service对象和method对象，绑定发送回调
- 调用service.callmethod,执行4步操作
- 框架给业务上报了请求参数,应用获取相应数据做本地业务;做本地业务;把相应写入(错误码，错误消息，返回值);执行回调操作(响应对象数据的序列化和网络发送)

Environment:
- muduo,boost==1.69.0
- protobuf==21.11
- zookeeper==3.4.10