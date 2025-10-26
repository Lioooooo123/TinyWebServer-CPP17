# C++17 现代化升级说明

## 概述

本项目已从原始的 C++98/03 代码升级到 **C++17** 标准，采用了现代 C++ 特性，提升了代码的安全性、可读性和可维护性。

## 主要升级内容

### 1. 编译标准升级

- **编译器标准**: 从隐式 C++98 升级到 **C++17** (`-std=c++17`)
- **编译选项**: 添加了 `-Wall -Wextra -Wpedantic` 以提升代码质量
- **Makefile**: 更新了构建配置

### 2. 核心类现代化

#### 2.1 Config 类改进

- **类型安全**: 移除 `using namespace std`，使用显式 `std::` 前缀
- **成员初始化**: 采用成员初始化列表进行构造
- **异常安全**: 改进参数解析，添加异常处理
- **新功能**: 支持从配置文件加载参数（使用 `-f` 选项）

**使用示例**:
```bash
# 从配置文件加载
./server -f server.conf

# 命令行参数依然可用（优先级更高）
./server -p 8080 -t 16
```

#### 2.2 WebServer 类改进

- **智能指针**: 线程池使用 `std::unique_ptr<threadpool<http_conn>>` 替代原始指针
- **RAII**: 成员采用 RAII 原则，资源自动管理
- **标准容器**: 
  - `users` 数组改为 `std::vector<http_conn>`
  - `users_timer` 数组改为 `std::vector<client_data>`
- **std::string**: `m_root` 等成员使用 `std::string` 替代 `char*`
- **std::filesystem**: 使用 C++17 filesystem 库处理路径

#### 2.3 线程池重构

- **std::thread**: 用 `std::vector<std::thread>` 替代 `pthread_t*`
- **Lambda**: 使用 lambda 表达式简化线程创建
- **RAII**: 析构函数自动 join 所有线程
- **移除静态worker**: 不再需要静态成员函数和 void* 转换

### 3. 同步原语现代化

#### 3.1 lock/locker.h 更新

- **std::mutex**: `locker` 类封装 `std::mutex` 替代 `pthread_mutex_t`
- **std::condition_variable**: `cond` 类封装条件变量
- **sem**: 信号量保留 POSIX 实现（C++20才有 `std::counting_semaphore`）

### 4. 配置文件支持

新增配置文件功能，支持从外部文件加载服务器配置。

**server.conf 示例**:
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

**命令行选项**:
- `-f <file>`: 从配置文件加载参数
- 命令行参数会覆盖配置文件中的设置

### 5. 类型安全改进

- 移除全局 `using namespace std`
- 所有 `string` 改为 `std::string`
- 所有 `map` 改为 `std::map`
- 函数参数使用 `const std::string&` 避免不必要的拷贝

## 向后兼容性

- ✅ 原有的命令行参数接口保持不变
- ✅ 数据库、日志、HTTP等模块接口保持兼容
- ✅ 配置文件为可选功能，不影响原有使用方式

## 编译要求

- **编译器**: g++ 7.0+ 或 clang++ 5.0+（支持 C++17）
- **依赖库**: 
  - pthread（线程支持）
  - mysqlclient（数据库连接）
  - C++17 标准库（filesystem）

## 构建与运行

```bash
# 清理旧构建
make clean

# 编译
make server

# 运行（使用配置文件）
./server -f server.conf

# 运行（使用命令行参数）
./server -p 9006 -t 8 -s 8
```

## 性能与优化

- **RAII**: 自动资源管理，减少内存泄漏风险
- **移动语义**: 容器使用 `emplace_back` 等就地构造
- **智能指针**: 线程池等资源自动释放
- **constexpr**: 编译期常量优化

## 已知限制

1. **信号量**: 仍使用 POSIX `sem_t`（C++20之前标准库无对应实现）
2. **日志模块**: 保留原有 pthread 实现（后续可升级）
3. **定时器**: 保留链表结构（后续可考虑 `std::priority_queue`）

## 未来改进方向

- [ ] 升级到 C++20（引入 `std::counting_semaphore`）
- [ ] 使用 `std::format` 替代 printf 风格格式化
- [ ] 引入 `std::span` 和 `std::string_view` 减少拷贝
- [ ] 使用协程（coroutines）重构异步I/O

## 贡献者

本次升级由 GitHub Copilot 完成，旨在将项目代码库现代化，提升代码质量和可维护性。

## 许可证

本项目遵循原项目的许可证条款。
