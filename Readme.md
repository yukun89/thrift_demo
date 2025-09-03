## 原理

在client端进行流量的录制与回放。

录制-record，实现在Transport层，核心在于对read/write方法的改造；其中我们主要关注read方法，可以不对write的流量进行录制。

回放-replay，也实现在Transport层。

## 文件生成
thrift --gen cpp SVR.thrift

## 编译

使用本目录下的Makefile编译

## 运行

client/server: 原始的thrift客户端/服务端

