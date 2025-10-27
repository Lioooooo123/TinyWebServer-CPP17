# 📢 C++17 现代化升级说明

## 🎉 重要更新

本项目已完成 **C++17 现代化升级**！采用了现代 C++ 特性，大幅提升了代码质量和可维护性。

### ✨ 主要改进

- ✅ **C++17 标准**: 全面升级到 C++17 编译标准
- ✅ **智能指针**: 使用 `std::unique_ptr` 管理资源
- ✅ **标准容器**: 采用 `std::vector` 替代原始数组
- ✅ **std::thread**: 用标准线程库替换 pthread
- ✅ **std::mutex/condition_variable**: 现代同步原语
- ✅ **配置文件支持**: 新增从文件加载配置功能

### 📦 新功能：配置文件支持

现在可以通过配置文件管理服务器参数：

```bash
# 使用配置文件启动
./server -f server.conf

# 配置文件示例（server.conf）
PORT=9006
thread_num=8
sql_num=8
LOGWrite=0
```

### 📚 详细说明

完整的升级文档请查看：[UPGRADE_NOTES.md](./UPGRADE_NOTES.md)

### 🔧 编译要求

- **编译器**: g++ 7.0+ 或 clang++ 5.0+
- **标准**: C++17
- **依赖**: pthread, mysqlclient

---

# TinyWebServer

Linux下C++轻量级Web服务器，助力初学者快速实践网络编程，搭建属于自己的服务器。

* 使用 **线程池 + 非阻塞socket + epoll(ET和LT均实现) + 事件处理(Reactor和模拟Proactor均实现)** 的并发模型
* 使用**状态机**解析HTTP请求报文，支持解析**GET和POST**请求
* 访问服务器数据库实现web端用户**注册、登录**功能，可以请求服务器**图片和视频文件**
* 实现**同步/异步日志系统**，记录服务器运行状态
* 经Webbench压力测试可以实现**上万的并发连接**数据交换
* **🆕 C++17现代化**: 使用智能指针、标准线程库、配置文件等现代特性

## 写在前面

* 本项目开发维护过程中，很多童鞋曾发红包支持，我都一一谢绝。我现在不会，将来也不会将本项目包装成任何课程售卖，更不会开通任何支持通道。
* 目前网络上有人或对本项目，或对游双大佬的项目包装成课程售卖。请各位童鞋擦亮眼，辨识各大学习/求职网站的C++服务器项目，不要盲目付费。
* 有面试官大佬通过项目信息在公司内找到我，发现很多童鞋简历上都用了这个项目。但，在面试过程中发现`很多童鞋通过本项目入门了，但是对于一些东西还是属于知其然不知其所以然的状态，需要加强下基础知识的学习`，推荐认真阅读下
    * 《unix环境高级编程》
    * 《unix网络编程》
* 感谢各位大佬，各位朋友，各位童鞋的认可和支持。如果本项目能带你入门，将是我莫大的荣幸。

## 目录

| [概述](#概述) | [框架](#框架) | [Demo演示](#Demo演示) | [压力测试](#压力测试) |[更新日志](#更新日志) |[源码下载](#源码下载) | [快速运行](#快速运行) | [个性化运行](#个性化运行) | [C++17升级](#C++17现代化升级) | [致谢](#致谢) |

## 概述

> * C/C++ (C++17)
> * B/S模型
> * [线程同步机制包装类](https://github.com/qinguoyi/TinyWebServer/tree/master/lock) (std::mutex/condition_variable)
> * [http连接请求处理类](https://github.com/qinguoyi/TinyWebServer/tree/master/http)
> * [半同步/半反应堆线程池](https://github.com/qinguoyi/TinyWebServer/tree/master/threadpool) (std::thread)
> * [定时器处理非活动连接](https://github.com/qinguoyi/TinyWebServer/tree/master/timer)
> * [同步/异步日志系统](https://github.com/qinguoyi/TinyWebServer/tree/master/log)  
> * [数据库连接池](https://github.com/qinguoyi/TinyWebServer/tree/master/CGImysql) 
> * [同步线程注册和登录校验](https://github.com/qinguoyi/TinyWebServer/tree/master/CGImysql) 
> * [简易服务器压力测试](https://github.com/qinguoyi/TinyWebServer/tree/master/test_presure)
> * 🆕 [配置文件支持](#配置文件) (可选)

## C++17现代化升级

本项目已全面升级到 C++17 标准，采用现代 C++ 最佳实践。详细升级说明请查看 [UPGRADE_NOTES.md](./UPGRADE_NOTES.md)。

### 主要特性

- **智能指针**: 线程池等资源使用 `std::unique_ptr` 自动管理
- **标准线程库**: 用 `std::thread`、`std::mutex`、`std::condition_variable` 替换 pthread
- **RAII**: 资源自动管理，减少内存泄漏风险
- **类型安全**: 使用 `std::string`、`std::vector` 等标准容器
- **配置文件**: 支持从外部文件加载服务器配置

### 配置文件

新增配置文件功能，方便管理服务器参数：

**创建 `server.conf`**:
```ini
# TinyWebServer 配置文件
PORT=9006
LOGWrite=0
TRIGMode=0
OPT_LINGER=0
sql_num=8
thread_num=8
close_log=0
actor_model=0
```

**使用配置文件启动**:
```bash
./server -f server.conf
```

命令行参数仍然可用，且优先级更高：
```bash
./server -f server.conf -p 8080 -t 16
```

## 快速运行

### 服务器测试环境
* Ubuntu 18.04+
* MySQL 5.7+
* C++ 17

### 编译与运行
```bash
# 清理旧构建
make clean

# 编译（需要 g++ 7.0+ 支持 C++17）
make server

# 使用配置文件运行
./server -f server.conf

# 或使用命令行参数运行
./server -p 9006 -t 8 -s 8
```

## 个性化运行

```bash
./server [-p port] [-l LOGWrite] [-m TRIGMode] [-o OPT_LINGER] 
         [-s sql_num] [-t thread_num] [-c close_log] [-a actor_model]
         [-f config_file]
```

参数说明：
* -p: 端口号（默认9006）
* -l: 日志写入方式，0同步，1异步（默认0）
* -m: 触发组合模式，0=LT+LT, 1=LT+ET, 2=ET+LT, 3=ET+ET（默认0）
* -o: 优雅关闭连接，0不使用，1使用（默认0）
* -s: 数据库连接数量（默认8）
* -t: 线程数量（默认8）
* -c: 关闭日志，0开启，1关闭（默认0）
* -a: 并发模型，0 Proactor，1 Reactor（默认0）
* **-f: 配置文件路径（新增）**

## 致谢

Linux高性能服务器编程，游双著.

感谢所有为本项目贡献的开发者和使用者！

---

**License**: 本项目遵循原项目的许可证条款
