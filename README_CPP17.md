# TinyWebServer C++17 现代化版本 🚀

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Lioooooo123/TinyWebServer-CPP17)

Linux下C++轻量级Web服务器的**现代化重构版本**，基于 [qinguoyi/TinyWebServer](https://github.com/qinguoyi/TinyWebServer) 进行 C++17 标准升级。

## ✨ 本版本特色

### 🔥 C++17 现代化升级
- **编译标准**: 使用 C++17 标准编译
- **智能指针**: `std::unique_ptr` 管理资源，自动内存管理
- **标准线程**: `std::thread` 替代 pthread，跨平台更好
- **标准容器**: `std::vector`、`std::string` 提升类型安全
- **RAII**: 资源获取即初始化，防止内存泄漏
- **std::filesystem**: 现代化的文件路径处理

### 🎯 新增功能
- **配置文件支持**: 通过 `-f` 参数加载配置文件
- **灵活配置**: 命令行参数可覆盖配置文件设置
- **错误处理**: 完善的异常处理和日志输出
- **类型安全**: 使用 `const std::string&` 避免不必要拷贝

### 📦 核心特性
* 使用 **线程池 + 非阻塞socket + epoll(ET和LT) + 事件处理(Reactor和Proactor)** 的并发模型
* 使用**状态机**解析HTTP请求报文，支持解析**GET和POST**请求
* 访问服务器数据库实现web端用户**注册、登录**功能，可以请求服务器**图片和视频文件**
* 实现**同步/异步日志系统**，记录服务器运行状态
* 经Webbench压力测试可以实现**上万的并发连接**数据交换

## 🆚 对比原版本

| 特性 | 原版本 | C++17版本 |
|------|--------|-----------|
| 编译标准 | C++98/03 | C++17 |
| 线程管理 | pthread | std::thread |
| 内存管理 | 手动 new/delete | std::unique_ptr |
| 容器 | 原始数组 | std::vector |
| 字符串 | char* | std::string |
| 同步原语 | pthread_mutex | std::mutex |
| 配置方式 | 仅命令行 | 命令行 + 配置文件 |
| 文件路径 | C 字符串操作 | std::filesystem |

## 🚀 快速开始

### 编译要求
- **编译器**: g++ 7.0+ 或 clang++ 5.0+ (支持 C++17)
- **依赖库**: pthread, mysqlclient

### 编译运行
```bash
# 克隆仓库
git clone https://github.com/Lioooooo123/TinyWebServer-CPP17.git
cd TinyWebServer-CPP17

# 编译
make clean
make server

# 使用配置文件运行
./server -f server.conf

# 使用命令行参数运行
./server -p 9006 -t 8 -s 8
```

### 配置文件示例 (server.conf)
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

### 数据库配置
```bash
# 创建数据库
create database yourdb;

# 创建用户表
USE yourdb;
CREATE TABLE user(
    username char(50) NULL,
    passwd char(50) NULL
)ENGINE=InnoDB;

# 插入测试数据
INSERT INTO user(username, passwd) VALUES('name', 'passwd');
```

修改 `main.cpp` 中的数据库信息：
```cpp
string user = "root";
string passwd = "root";
string databasename = "yourdb";
```

## 📝 命令行参数

```bash
./server [-p port] [-l LOGWrite] [-m TRIGMode] [-o OPT_LINGER] 
         [-s sql_num] [-t thread_num] [-c close_log] [-a actor_model]
         [-f config_file]
```

参数说明：
- `-p`: 端口号，默认9006
- `-l`: 日志写入方式，0=同步，1=异步，默认0
- `-m`: 触发模式，0=LT+LT，1=LT+ET，2=ET+LT，3=ET+ET，默认0
- `-o`: 优雅关闭连接，0=不使用，1=使用，默认0
- `-s`: 数据库连接池数量，默认8
- `-t`: 线程池内线程数量，默认8
- `-c`: 关闭日志，0=开启，1=关闭，默认0
- `-a`: 并发模型，0=proactor，1=reactor，默认0
- `-f`: 配置文件路径（**新增**）

## 📚 项目结构

```
.
├── CGImysql/          # 数据库连接池
├── http/              # HTTP请求处理
├── lock/              # 同步原语封装 (现代化为 std::mutex)
├── log/               # 日志系统
├── threadpool/        # 线程池 (现代化为 std::thread)
├── timer/             # 定时器
├── root/              # 网站资源文件
├── test_pressure/     # 压力测试工具
├── config.h/cpp       # 配置管理 (新增文件加载功能)
├── webserver.h/cpp    # 服务器核心 (现代化实现)
├── main.cpp           # 主程序
├── makefile           # 构建文件 (C++17)
├── server.conf        # 配置文件示例 (新增)
├── UPGRADE_NOTES.md   # 升级说明 (新增)
└── COMPLETION_REPORT.md # 完成报告 (新增)
```

## 🔧 技术细节

### 线程池实现
```cpp
// 使用 std::thread 和 lambda
for (int i = 0; i < thread_number; ++i)
{
    m_threads.emplace_back([this]() { this->run(); });
}
```

### 智能指针管理
```cpp
// 线程池使用智能指针自动管理
std::unique_ptr<threadpool<http_conn>> m_pool;
m_pool = std::make_unique<threadpool<http_conn>>(...);
```

### 配置文件加载
```cpp
// 新增配置文件加载功能
Config config;
config.load_from_file("server.conf");
config.parse_arg(argc, argv);  // 命令行参数优先级更高
```

## 📖 学习资源

### 推荐阅读
- [UPGRADE_NOTES.md](UPGRADE_NOTES.md) - 详细的升级说明
- [COMPLETION_REPORT.md](COMPLETION_REPORT.md) - 完整的升级报告
- 《C++ Primer》第5版 - C++11/14 特性
- 《Effective Modern C++》 - 现代 C++ 最佳实践

### 原项目资源
- [原项目地址](https://github.com/qinguoyi/TinyWebServer)
- [原项目文档](https://github.com/qinguoyi/TinyWebServer/tree/master)

## 🎯 适用人群

- 正在学习现代 C++ 的开发者
- 想了解服务器开发的初学者
- 希望了解 C++ 代码现代化升级的工程师
- 准备面试 C++ 后端岗位的同学

## 🐛 已知问题

- 部分日志模块仍有编译警告（不影响功能）
- 信号量仍使用 POSIX 实现（C++20 才有标准实现）

## 🚧 未来计划

- [ ] 升级到 C++20（协程、模块、概念）
- [ ] 使用 `std::format` 替代 printf
- [ ] 完善单元测试
- [ ] 性能优化和基准测试

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

## 📄 许可证

本项目遵循原项目的许可证条款。

## 🙏 致谢

- 感谢 [qinguoyi](https://github.com/qinguoyi) 的优秀原项目
- 感谢所有为本项目提供建议的开发者

## 📮 联系方式

- GitHub: [@Lioooooo123](https://github.com/Lioooooo123)
- Email: lioooooo123@example.com

---

⭐ 如果这个项目对你有帮助，欢迎 Star！

**最后更新**: 2025年10月26日
