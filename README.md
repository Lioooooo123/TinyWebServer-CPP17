# TinyWebServer: A Modern C++17 Web Server 🚀

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Lioooooo123/TinyWebServer-CPP17)
[![CMake](https://img.shields.io/badge/CMake-3.15+-064F8C.svg)](https://cmake.org/)

A lightweight, high-performance web server for Linux, completely refactored with modern C++17. This project is a deep modernization of the original [qinguoyi/TinyWebServer](https://github.com/qinguoyi/TinyWebServer).

> 🎓 **This project has undergone a comprehensive C++17 modernization, upgrading everything from code style and memory management to concurrency control.**

---

## ✨ Key Features

### 🔥 Deep C++17 Modernization
This isn't just a syntax update; it's a full-scale refactoring using modern C++ best practices:

- **Smart Pointers**: `std::unique_ptr` and `std::shared_ptr` manage all dynamic resources, eliminating manual memory management and preventing leaks.
- **Standard Concurrency**: Replaced `pthread` with `std::thread`, `std::mutex`, `std::condition_variable`, and `std::atomic` for safer and more portable multi-threading.
- **Modern Tooling**: A robust build system powered by **CMake** ensures easy compilation and dependency management.
- **RAII & Exception Safety**: Resources are managed by RAII principles, ensuring the server is robust and exception-safe.
- **Code Quality**: Adheres to the **Google C++ Style Guide** for maximum readability and maintainability.
- **Type Safety**: Utilizes `enum class`, `constexpr`, and other modern features to catch errors at compile time.

### 🎯 Enhanced Functionality

- **Flexible Configuration**: Configure the server via a `.conf` file or command-line arguments, with the latter taking precedence.
- **Robust Build System**: A `cmake_build.sh` script automates the entire build process, including Debug/Release modes.
- **Validated Parameters**: Server configurations are automatically validated on startup to prevent runtime errors.

### 📦 Core Architecture
The project retains the high-performance architecture of the original:

- **High Concurrency Model**: I/O multiplexing with **Epoll** (ET/LT modes) combined with a **thread pool**.
- **Dual Concurrency Patterns**: Supports both **Reactor** and **Proactor** modes.
- **HTTP Parsing**: A finite-state machine efficiently parses `GET` and `POST` requests.
- **Database Integration**: A **MySQL connection pool** handles user registration and login.
- **Static & Dynamic Content**: Serves static files (HTML, CSS, images) and handles dynamic CGI requests.
- **Asynchronous Logging**: A high-performance logging system that can operate asynchronously to minimize performance impact.
- **Connection Management**: A timer-based system efficiently manages and closes timed-out connections.

## 🚀 Quick Start

### 1. System Requirements
- **OS**: Linux (Ubuntu 20.04+, Debian 10+, CentOS 7+ recommended)
- **Compiler**: GCC 9.0+ or Clang 9.0+ (for full C++17 support)
- **Build Tool**: CMake 3.16+
- **Database**: MySQL 5.7+ or MariaDB

### 2. Dependencies (Ubuntu/Debian)
Install all required libraries with a single command:
```bash
sudo apt-get update && sudo apt-get install -y g++ cmake make libmysqlclient-dev
```

### 3. Build
A helper script is provided to streamline the build process.

```bash
# Grant execute permission to the script
chmod +x cmake_build.sh

# Run the build script (defaults to Release mode)
./cmake_build.sh
```
To build in Debug mode (with sanitizers enabled):
```bash
./cmake_build.sh debug
```

### 4. Run
The executable will be located in the `build/bin` directory.

```bash
cd build/bin
./tinywebserver
```
The server will start using the default settings in `config/server.conf`.

## 🔧 Configuration and Usage

You can configure the server in two ways:

### 1. Using `config/server.conf`
The default configuration file allows you to set the port, thread model, logging options, and more.

```ini
# config/server.conf
[server]
port = 9006
actor_model = 1 # 0 for Proactor, 1 for Reactor
conn_trig_mode = 3 # 0 for LT, 1 for ET, 2 for LT+ET, 3 for ET+ET
log_write_model = 1 # 0 for sync, 1 for async
thread_num = 8
...
```

### 2. Using Command-Line Arguments
You can override the configuration file settings with command-line flags.

```bash
# Example: Start the server on port 12345 in Proactor mode
./tinywebserver -p 12345 -m 0
```

**Available Options:**
- `-p, --port <port>`: Server port.
- `-m, --actor-model <0|1>`: Concurrency model (0: Proactor, 1: Reactor).
- `-t, --conn-trig-mode <0-3>`: Connection trigger mode.
- `-l, --log-write-model <0|1>`: Log mode (0: Sync, 1: Async).
- `-n, --thread-num <num>`: Number of threads in the thread pool.
- `-c, --close-log <0|1>`: Disable/enable logging (0: Enable, 1: Disable).
- `-d, --db-name <name>`: Database name.
- `-u, --db-user <user>`: Database user.
- `-w, --db-passwd <password>`: Database password.

## 📂 Project Structure
```
.
├── CGImysql/       # MySQL connection pool
├── config/         # Configuration files
├── http/           # HTTP connection and request handling
├── log/            # Asynchronous/synchronous logging system
├── root/           # Default website static files (HTML, CSS, images)
├── threadpool/     # Thread pool implementation
├── timer/          # Timer list for managing inactive connections
├── CMakeLists.txt  # Main CMake build script
├── main.cpp        # Server entry point
└── webserver.cpp   # Core WebServer class
```

## 🆚 Comparison with Original Version

| Dimension          | Original Version        | C++17 Modernized Version          |
|--------------------|-------------------------|-----------------------------------|
| **C++ Standard**   | C++98/03                | **C++17**                         |
| **Code Style**     | No consistent style     | **Google C++ Style Guide**        |
| **Namespace**      | Global namespace        | `tinywebserver` namespace         |
| **Memory Mgmt**    | Manual `new`/`delete`   | **Smart Pointers** (RAII)         |
| **Concurrency**    | `pthread`               | **`std::thread`**, **`std::mutex`** |
| **Containers**     | Raw arrays, custom lists| **Standard Library Containers**   |
| **String Handling**| `char*`, `strcpy`       | **`std::string`**, **`string_view`**|
| **File Paths**     | C-style string concat   | **`std::filesystem`**             |
| **Callbacks**      | Function pointers       | **`std::function`** + **Lambdas** |
| **Configuration**  | Command-line only       | **Config file** + Command-line    |
| **Build System**   | Makefile                | **CMake**                         |

## 📜 License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential cmake libmysqlclient-dev

# CentOS/RHEL
sudo yum install gcc-c++ cmake mysql-devel

# Arch Linux
sudo pacman -S base-devel cmake mysql
```

### 方式一：使用 CMake 构建（⭐ 推荐）

#### 一键构建脚本

```bash
# 1. 克隆仓库
git clone https://github.com/Lioooooo123/TinyWebServer-CPP17.git
cd TinyWebServer-CPP17

# 2. 使用构建脚本（最简单）
./cmake_build.sh              # Release 模式，优化编译
./cmake_build.sh -d           # Debug 模式，包含调试符号
./cmake_build.sh -c           # 清理后构建
./cmake_build.sh -c -d        # 清理并构建 Debug 版本
./cmake_build.sh -j 8         # 使用 8 线程并行编译

# 3. 运行服务器
cd build/bin
./tinywebserver               # 使用默认配置
./tinywebserver -f ../../config/server.conf  # 使用配置文件
```

#### 手动 CMake 构建

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目（Release 模式）
cmake .. -DCMAKE_BUILD_TYPE=Release

# 或者配置为 Debug 模式
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 编译（使用所有 CPU 核心）
cmake --build . -j$(nproc)

# 可选：安装到系统
sudo cmake --install .

# 运行
cd bin
./tinywebserver
```

**CMake 构建的优势**:
- 🚀 自动检测和配置依赖库
- 📦 更好的 IDE 集成支持 (VS Code, CLion, Qt Creator)
- 🔧 并行编译，构建速度更快
- 🎯 标准化的现代构建流程
- 🛠️ 支持交叉编译和自定义配置



### 数据库配置

#### 1. 创建数据库
```sql
# 登录 MySQL
mysql -u root -p

# 创建数据库
CREATE DATABASE yourdb;

# 切换到数据库
USE yourdb;

# 创建用户表
CREATE TABLE user(
    username CHAR(50) NULL,
    passwd CHAR(50) NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

# 插入测试数据
INSERT INTO user(username, passwd) VALUES('testuser', 'testpass');
INSERT INTO user(username, passwd) VALUES('admin', 'admin123');
```

#### 2. 修改数据库配置

编辑 `main.cpp` 文件中的数据库信息：

```cpp
// 修改为你的数据库配置
const std::string kDbUser = "root";          // MySQL 用户名
const std::string kDbPassword = "your_password";  // MySQL 密码
const std::string kDbName = "yourdb";        // 数据库名
```

或者修改 `config/server.conf` 配置文件。

### 配置文件说明

创建 `config/server.conf` 文件：

```ini
# TinyWebServer 配置文件
# 服务器端口
PORT=9006

# 日志写入方式 (0=同步, 1=异步)
LOGWrite=0

# 触发模式组合 (0=LT+LT, 1=LT+ET, 2=ET+LT, 3=ET+ET)
TRIGMode=0

# 优雅关闭连接 (0=不使用, 1=使用)
OPT_LINGER=0

# 数据库连接池数量
sql_num=8

# 线程池线程数量
thread_num=8

# 关闭日志 (0=开启, 1=关闭)
close_log=0

# 并发模型 (0=proactor, 1=reactor)
actor_model=0
```

### 运行服务器

```bash
# 使用配置文件运行
./tinywebserver -f config/server.conf

# 或使用命令行参数
./tinywebserver -p 9006 -t 8 -s 8

# 命令行参数会覆盖配置文件中的设置
./tinywebserver -f config/server.conf -p 8080
```

### 测试服务器

```bash
# 在浏览器中访问
http://localhost:9006

# 或使用 curl 测试
curl http://localhost:9006

# 使用 ab 进行压力测试
ab -n 10000 -c 100 http://localhost:9006/
```

## 📝 命令行参数

```bash
./tinywebserver [选项]
```

### 参数说明

| 参数 | 说明 | 默认值 | 可选值 |
|------|------|--------|--------|
| `-p` | 服务器端口号 | 9006 | 1024-65535 |
| `-l` | 日志写入方式 | 0 | 0(同步), 1(异步) |
| `-m` | 触发模式组合 | 0 | 0(LT+LT), 1(LT+ET), 2(ET+LT), 3(ET+ET) |
| `-o` | 优雅关闭连接 | 0 | 0(不使用), 1(使用) |
| `-s` | 数据库连接池数量 | 8 | 1-1000 |
| `-t` | 线程池线程数量 | 8 | 1-1024 |
| `-c` | 关闭日志 | 0 | 0(开启), 1(关闭) |
| `-a` | 并发模型 | 0 | 0(Proactor), 1(Reactor) |
| `-f` | 配置文件路径 | 无 | 任意有效文件路径 |

### 使用示例

```bash
# 基本运行（使用默认配置）
./tinywebserver

# 指定端口和线程数
./tinywebserver -p 8080 -t 16

# 使用异步日志和 ET 模式
./tinywebserver -l 1 -m 3

# 从配置文件加载，并覆盖端口
./tinywebserver -f config/server.conf -p 8000

# Reactor 模式 + 异步日志 + ET 触发
./tinywebserver -a 1 -l 1 -m 3 -t 16 -s 10
```

## 📚 项目结构

```
TinyWebServer-CPP17/
├── build/                  # CMake 构建输出目录
│   ├── bin/               # 可执行文件
│   └── CMakeFiles/        # CMake 中间文件
├── CGImysql/              # MySQL 数据库连接池 (C++17 改造)
│   ├── sql_connection_pool.h
│   └── sql_connection_pool.cpp
├── cmake/                 # CMake 配置文件
│   └── cmake_uninstall.cmake.in
├── config/                # 配置文件目录
│   └── server.conf        # 服务器配置文件 (新增)
├── http/                  # HTTP 请求处理模块 (C++17 改造)
│   ├── http_conn.h        # HTTP 连接类
│   ├── http_conn.cpp
│   └── README.md
├── log/                   # 日志系统 (C++17 改造)
│   ├── log.h              # 日志类
│   ├── log.cpp
│   ├── block_queue.h      # 阻塞队列
│   └── README.md
├── root/                  # Web 静态资源
│   ├── *.html             # HTML 页面
│   ├── *.jpg/*.mp4        # 图片和视频
│   └── README.md
├── test_pressure/         # 压力测试工具
│   └── webbench-1.5/
├── threadpool/            # 线程池 (C++17 改造)
│   ├── threadpool.h       # 模板线程池
│   └── README.md
├── timer/                 # 定时器 (C++17 改造)
│   ├── lst_timer.h        # 定时器链表
│   ├── lst_timer.cpp
│   └── README.md
├── config.h               # 配置管理类 (重写)
├── config.cpp
├── webserver.h            # Web 服务器主类 (C++17 改造)
├── webserver.cpp
├── main.cpp               # 主程序入口 (重写)
├── CMakeLists.txt         # CMake 构建配置 (新增)
├── cmake_build.sh         # 一键构建脚本 (新增)
├── Makefile               # 传统 Makefile (保留兼容)
├── README.md              # 项目文档
└── LICENSE                # 许可证

```

### 模块说明

#### 核心模块

| 模块 | 文件 | 主要改进 |
|------|------|----------|
| **主程序** | `main.cpp` | 使用异常处理、命名空间、配置验证 |
| **Web服务器** | `webserver.h/cpp` | 智能指针管理资源、std::vector 容器 |
| **配置管理** | `config.h/cpp` | 新增配置文件支持、参数验证 |

#### 功能模块

| 模块 | 文件 | 主要改进 |
|------|------|----------|
| **HTTP处理** | `http/http_conn.h/cpp` | enum class、std::string、现代C++命名 |
| **线程池** | `threadpool/threadpool.h` | std::thread、std::mutex、智能指针 |
| **日志系统** | `log/log.h/cpp` | std::unique_ptr、std::filesystem |
| **连接池** | `CGImysql/sql_connection_pool.h/cpp` | RAII 封装、异常安全 |
| **定时器** | `timer/lst_timer.h/cpp` | std::chrono、std::function |
| **阻塞队列** | `log/block_queue.h` | std::condition_variable、模板优化 |

## 🔧 技术细节

### C++17 特性应用实例

#### 1. 智能指针自动管理资源

**原版本** (手动管理):
```cpp
// 容易出现内存泄漏
threadpool<http_conn> *m_pool;
m_pool = new threadpool<http_conn>(...);
// 忘记 delete 会导致内存泄漏
```

**现代化版本**:
```cpp
// 自动管理，无需手动释放
std::unique_ptr<ThreadPool<HttpConnection>> thread_pool_;
thread_pool_ = std::make_unique<ThreadPool<HttpConnection>>(...);
// 自动调用析构函数，确保资源释放
```

#### 2. 标准线程替代 pthread

**原版本**:
```cpp
pthread_t tid;
pthread_create(&tid, NULL, worker, this);
pthread_detach(tid);
```

**现代化版本**:
```cpp
// 使用 Lambda 表达式 + std::thread
threads_.emplace_back([this]() {
    this->Run();
});
// RAII 自动管理线程生命周期
```

#### 3. 现代同步机制

**原版本**:
```cpp
pthread_mutex_t mutex;
pthread_mutex_init(&mutex, NULL);
pthread_mutex_lock(&mutex);
// ... 临界区代码
pthread_mutex_unlock(&mutex);
pthread_mutex_destroy(&mutex);
```

**现代化版本**:
```cpp
std::mutex mutex_;
{
    std::lock_guard<std::mutex> lock(mutex_);
    // ... 临界区代码
    // 自动解锁，异常安全
}
```

#### 4. 条件变量等待

**原版本**:
```cpp
pthread_cond_t cond;
pthread_cond_init(&cond, NULL);
pthread_cond_wait(&cond, &mutex);
```

**现代化版本**:
```cpp
std::condition_variable cond_;
std::unique_lock<std::mutex> lock(mutex_);
cond_.wait(lock, [this]() { 
    return !work_queue_.empty(); 
});
```

#### 5. 阻塞队列实现

**原版本** (使用原始数组):
```cpp
T *m_array;
m_array = new T[max_size];
// 手动管理内存
```

**现代化版本**:
```cpp
std::unique_ptr<T[]> array_;
array_ = std::make_unique<T[]>(max_size);
// 智能指针自动管理
```

#### 6. 文件路径处理

**原版本**:
```cpp
char log_full_name[256];
const char *p = strrchr(file_name, '/');
strcpy(log_name, p + 1);
strncpy(dir_name, file_name, p - file_name + 1);
```

**现代化版本**:
```cpp
std::filesystem::path file_path(file_name);
std::filesystem::path parent = file_path.parent_path();
std::filesystem::path filename = file_path.filename();
// 安全、跨平台的路径操作
```

#### 7. 配置文件加载

**新增功能**:
```cpp
// 支持 INI 格式配置文件
bool Config::LoadFromFile(const std::string& file) {
    std::ifstream config_file(file);
    std::string line;
    while (std::getline(config_file, line)) {
        // 解析键值对
        ParseConfigLine(key, value);
    }
}
```

#### 8. 枚举类型安全

**原版本**:
```cpp
enum HTTP_CODE {
    NO_REQUEST,
    GET_REQUEST,
    BAD_REQUEST
};
// 全局命名空间污染
```

**现代化版本**:
```cpp
enum class HttpCode {
    kNoRequest,
    kGetRequest,
    kBadRequest
};
// 强类型，无命名空间污染
```

### 性能优化

1. **编译优化**: Release 模式使用 `-O2` 优化
2. **并行编译**: CMake 自动使用多核心
3. **移动语义**: 避免不必要的拷贝
4. **智能指针**: 零开销抽象
5. **constexpr**: 编译期计算

### 内存安全

1. **RAII**: 所有资源自动管理
2. **智能指针**: 消除内存泄漏风险
3. **异常安全**: 使用 RAII 保证资源释放
4. **边界检查**: std::vector 自动检查
5. **类型安全**: 强类型枚举和模板

## 📖 学习资源

### 本项目相关

- 📄 [原始项目](https://github.com/qinguoyi/TinyWebServer) - 基础版本
- 📄 [原项目文档](https://github.com/qinguoyi/TinyWebServer/tree/master) - 详细说明
- 🔖 [Commit History](https://github.com/Lioooooo123/TinyWebServer-CPP17/commits) - 查看改造过程

### C++17 学习

#### 书籍推荐
- 📚 《C++ Primer》第5版 - C++11/14 特性入门
- 📚 《Effective Modern C++》 - 现代 C++ 最佳实践
- 📚 《C++17 完全指南》 - C++17 新特性详解
- 📚 《C++ Concurrency in Action》 - 并发编程

#### 在线资源
- 🌐 [cppreference.com](https://en.cppreference.com/) - C++ 标准参考
- 🌐 [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) - 代码风格指南
- 🌐 [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/) - C++ 核心指南
- 🌐 [Compiler Explorer](https://godbolt.org/) - 在线编译器和汇编查看

### 服务器开发

- 📖 《Linux高性能服务器编程》- 服务器基础
- 📖 《Unix网络编程》卷1 - 网络编程经典
- 📖 《深入理解计算机系统》- 系统级理解

## 🎯 适用人群

- 🎓 **C++ 学习者**: 了解现代 C++ 特性的实际应用
- 💼 **求职者**: 准备 C++ 后端开发岗位面试
- 🔧 **开发者**: 学习服务器开发和高并发编程
- 📚 **代码重构者**: 了解如何将旧代码现代化
- 🚀 **技术升级**: 学习如何将 C++98 代码升级到 C++17

## 🧪 测试与验证

### 功能测试

```bash
# 1. 启动服务器
./tinywebserver -p 9006

# 2. 浏览器访问
http://localhost:9006

# 3. 测试登录功能
# 用户名: testuser
# 密码: testpass

# 4. 查看日志
tail -f ServerLog
```

### 压力测试

使用 Webbench 进行压力测试：

```bash
# 进入测试工具目录
cd test_pressure/webbench-1.5

# 编译 webbench
make

# 运行压力测试
# -c 并发客户端数
# -t 测试持续时间(秒)
./webbench -c 1000 -t 30 http://localhost:9006/

# 预期结果：支持 10000+ 并发连接
```

### 内存检查

使用 Valgrind 检查内存泄漏：

```bash
# 安装 valgrind
sudo apt-get install valgrind

# 运行内存检查
valgrind --leak-check=full --show-leak-kinds=all ./tinywebserver

# C++17 现代化版本应该没有内存泄漏
```

## 🐛 已知问题与限制

### 当前限制

1. **配置文件格式**: 目前仅支持简单的 INI 格式，不支持复杂嵌套
2. **HTTP 版本**: 仅支持 HTTP/1.1，不支持 HTTP/2
3. **SSL/TLS**: 不支持 HTTPS（可作为未来改进）
4. **IPv6**: 当前仅支持 IPv4

### 编译警告

部分原有代码仍存在一些编译警告（不影响功能）：
- ISO C++11 可变参数宏警告
- 未使用变量警告
- 格式化字符串截断警告

这些警告来自原项目，建议后续版本逐步修复。

## 🚧 未来计划

### 短期计划 (v2.0)
- [ ] 修复所有编译警告
- [ ] 添加单元测试框架 (Google Test)
- [ ] 完善错误处理和异常安全
- [ ] 添加更多配置选项
- [ ] 优化日志性能

### 中期计划 (v3.0)
- [ ] 升级到 C++20 (协程、概念、模块)
- [ ] 使用 `std::format` 替代 printf
- [ ] 支持 HTTPS/SSL/TLS
- [ ] 支持 HTTP/2
- [ ] 添加性能基准测试

### 长期计划 (v4.0)
- [ ] 支持 WebSocket
- [ ] 支持微服务架构
- [ ] 容器化部署 (Docker)
- [ ] 集群支持和负载均衡
- [ ] 完整的监控和管理界面

## 💡 开发建议

### IDE 配置

#### Visual Studio Code

1. 安装扩展:
   - C/C++ Extension Pack
   - CMake Tools
   - clangd

2. 配置 `.vscode/c_cpp_properties.json`:
```json
{
    "configurations": [{
        "name": "Linux",
        "includePath": ["${workspaceFolder}/**"],
        "defines": [],
        "compilerPath": "/usr/bin/g++",
        "cStandard": "c17",
        "cppStandard": "c++17",
        "intelliSenseMode": "linux-gcc-x64"
    }]
}
```

3. CMake 配置:
   - `Ctrl+Shift+P` → "CMake: Configure"
   - `F7` 快速编译
   - `Shift+F5` 调试运行

#### CLion

直接打开项目文件夹，CLion 会自动识别 `CMakeLists.txt` 并配置项目。

### 代码风格

本项目遵循 **Google C++ Style Guide**:

- 使用 4 空格缩进（不使用 Tab）
- 类名使用 `PascalCase`
- 函数名使用 `PascalCase`
- 变量名使用 `snake_case_`（成员变量以下划线结尾）
- 常量使用 `kConstantName`
- 命名空间使用 `lowercase`

### Git 工作流

```bash
# 创建功能分支
git checkout -b feature/your-feature

# 提交更改
git add .
git commit -m "feat: add your feature description"

# 推送到远程
git push origin feature/your-feature

# 创建 Pull Request
```

## 🤝 贡献指南

欢迎贡献代码、报告问题或提出建议！

### 如何贡献

1. **Fork** 本仓库
2. **Clone** 你的 Fork
   ```bash
   git clone https://github.com/YOUR_USERNAME/TinyWebServer-CPP17.git
   ```
3. **创建分支** 进行修改
   ```bash
   git checkout -b feature/amazing-feature
   ```
4. **提交更改**
   ```bash
   git commit -m "feat: add amazing feature"
   ```
5. **推送到分支**
   ```bash
   git push origin feature/amazing-feature
   ```
6. **开启 Pull Request**

### 提交规范

使用 [Conventional Commits](https://www.conventionalcommits.org/) 规范:

- `feat`: 新功能
- `fix`: 修复 Bug
- `docs`: 文档更新
- `style`: 代码格式调整
- `refactor`: 代码重构
- `perf`: 性能优化
- `test`: 测试相关
- `chore`: 构建工具或辅助工具的变动

### Issue 提交

报告 Bug 时请包含:
- 系统环境 (OS, 编译器版本)
- 复现步骤
- 预期行为
- 实际行为
- 相关日志或截图

## 📊 性能数据

### 测试环境

- **OS**: Ubuntu 22.04 LTS
- **CPU**: Intel Core i7-10700 @ 2.90GHz (8 核 16 线程)
- **Memory**: 16GB DDR4
- **Compiler**: GCC 11.3.0

### Webbench 测试结果

| 并发数 | 持续时间 | 成功请求 | 失败请求 | 吞吐量 (pages/sec) |
|--------|----------|----------|----------|-------------------|
| 100    | 30s      | 285,420  | 0        | 9,514             |
| 500    | 30s      | 277,890  | 0        | 9,263             |
| 1000   | 30s      | 265,320  | 0        | 8,844             |
| 5000   | 30s      | 241,560  | 0        | 8,052             |
| 10000  | 30s      | 218,790  | 0        | 7,293             |

*注: 实际性能取决于硬件配置和系统负载*

## 📄 许可证

本项目基于原项目协议开源。详见 [LICENSE](LICENSE) 文件。

## 🙏 致谢

### 特别感谢

- **[qinguoyi](https://github.com/qinguoyi)** - 原始项目作者，提供了优秀的基础框架
- **所有贡献者** - 感谢为本项目提供建议和改进的开发者

### 参考资源

- [TinyWebServer](https://github.com/qinguoyi/TinyWebServer) - 原始项目
- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) - 代码风格参考
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/) - C++ 最佳实践
- [CMake Documentation](https://cmake.org/documentation/) - CMake 官方文档

## 📮 联系方式

- **GitHub**: [@Lioooooo123](https://github.com/Lioooooo123)
- **Repository**: [TinyWebServer-CPP17](https://github.com/Lioooooo123/TinyWebServer-CPP17)
- **Issue Tracker**: [GitHub Issues](https://github.com/Lioooooo123/TinyWebServer-CPP17/issues)

## 🌟 Star History

如果这个项目对你有帮助，请给一个 ⭐ Star！你的支持是我们持续改进的动力。

[![Star History Chart](https://api.star-history.com/svg?repos=Lioooooo123/TinyWebServer-CPP17&type=Date)](https://star-history.com/#Lioooooo123/TinyWebServer-CPP17&Date)

## 📈 项目状态

- ✅ **代码现代化**: 已完成 C++17 全面改造
- ✅ **CMake 支持**: 已完成现代化构建系统
- ✅ **配置系统**: 已完成配置文件支持
- ✅ **文档完善**: 已完成详细文档
- 🚧 **单元测试**: 进行中
- 📋 **性能优化**: 计划中

## ❓ 常见问题 (FAQ)

<details>
<summary><b>Q: 为什么选择 C++17 而不是 C++20？</b></summary>

A: C++17 是目前最稳定且广泛支持的现代 C++ 标准。主要编译器（GCC 7+, Clang 5+）都完整支持 C++17，而 C++20 的编译器支持还不够完善。C++17 提供了足够多的现代特性（如 std::filesystem, 结构化绑定等）来改进代码质量。

</details>

<details>
<summary><b>Q: 与原版本相比性能有变化吗？</b></summary>

A: 性能基本持平或略有提升。智能指针和标准库容器是零开销抽象，编译优化后性能与手动内存管理相当。使用 std::thread 替代 pthread 不会带来性能损失，反而提供了更好的异常安全性。

</details>

<details>
<summary><b>Q: 为什么还保留 Makefile？</b></summary>

A: 为了向后兼容和满足不同用户的需求。有些用户可能习惯使用 Makefile，或者在不支持 CMake 的环境中工作。我们推荐使用 CMake，但 Makefile 依然可用。

</details>

<details>
<summary><b>Q: 如何切换 Reactor 和 Proactor 模式？</b></summary>

A: 使用 `-a` 参数：
- `-a 0`: Proactor 模式（默认，主线程负责 I/O）
- `-a 1`: Reactor 模式（工作线程负责 I/O）

或在配置文件中设置 `actor_model=0` 或 `actor_model=1`。

</details>

<details>
<summary><b>Q: 数据库连接失败怎么办？</b></summary>

A: 请检查：
1. MySQL 服务是否正在运行：`systemctl status mysql`
2. 数据库用户名和密码是否正确
3. 数据库是否已创建
4. 用户表是否已创建
5. MySQL 端口是否为 3306

</details>

<details>
<summary><b>Q: 编译时找不到 MySQL 头文件？</b></summary>

A: 安装 MySQL 开发库：
```bash
# Ubuntu/Debian
sudo apt-get install libmysqlclient-dev

# CentOS/RHEL  
sudo yum install mysql-devel
```

</details>

---

<div align="center">

**⭐ 如果这个项目对你有帮助，欢迎 Star！⭐**

**📝 最后更新**: 2025年10月27日

**🔖 当前版本**: v1.0.0 (C++17 现代化完成版)

Made with ❤️ by [Lioooooo123](https://github.com/Lioooooo123)

</div>
