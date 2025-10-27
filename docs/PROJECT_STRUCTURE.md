# 项目目录结构

```
TinyWebServer-CPP17/
│
├── CGImysql/                    # 数据库连接池模块
│   ├── sql_connection_pool.h   # 连接池头文件
│   ├── sql_connection_pool.cpp # 连接池实现
│   └── README.md               # 模块说明
│
├── http/                        # HTTP 请求处理模块
│   ├── http_conn.h             # HTTP连接类头文件
│   ├── http_conn.cpp           # HTTP连接类实现
│   └── README.md               # 模块说明
│
├── lock/                        # 线程同步机制封装 (C++17现代化)
│   ├── locker.h                # 互斥锁、条件变量、信号量封装
│   └── README.md               # 模块说明
│
├── log/                         # 日志系统模块
│   ├── log.h                   # 日志类头文件
│   ├── log.cpp                 # 日志类实现
│   ├── block_queue.h           # 阻塞队列（异步日志用）
│   └── README.md               # 模块说明
│
├── threadpool/                  # 线程池模块 (C++17现代化)
│   ├── threadpool.h            # 线程池模板类
│   └── README.md               # 模块说明
│
├── timer/                       # 定时器模块
│   ├── lst_timer.h             # 定时器链表头文件
│   ├── lst_timer.cpp           # 定时器链表实现
│   └── README.md               # 模块说明
│
├── root/                        # Web 资源文件
│   ├── *.html                  # HTML 页面
│   ├── *.gif                   # 图片资源
│   ├── *.jpg                   # 图片资源
│   ├── *.mp4                   # 视频资源
│   └── README.md               # 资源说明
│
├── test_pressure/               # 压力测试工具
│   ├── webbench-1.5/           # webbench 工具
│   └── README.md               # 测试说明
│
├── config/                      # 配置文件目录 (新增)
│   └── server.conf             # 服务器配置文件
│
├── docs/                        # 文档目录 (新增)
│   ├── QUICKSTART.md           # 快速开始指南
│   ├── UPGRADE_NOTES.md        # C++17升级说明
│   └── COMPLETION_REPORT.md    # 升级完成报告
│
├── config.h                     # 配置类头文件 (支持配置文件加载)
├── config.cpp                   # 配置类实现
├── webserver.h                  # 服务器核心类头文件 (C++17现代化)
├── webserver.cpp                # 服务器核心类实现
├── main.cpp                     # 主程序入口
├── makefile                     # 构建文件 (C++17标准)
├── build.sh                     # 快速编译脚本
│
├── .gitignore                   # Git忽略配置
├── LICENSE                      # 许可证文件
├── README.md                    # 项目说明（原版）
└── README_CPP17.md              # C++17版本说明 (新增)
```

## 核心模块说明

### 1. 主程序入口
- `main.cpp`: 程序启动入口，初始化各模块并启动服务器

### 2. 服务器核心
- `webserver.h/cpp`: 服务器主类，整合所有模块
  - 使用 `std::unique_ptr` 管理线程池
  - 使用 `std::vector` 管理连接和定时器
  - 使用 `std::string` 处理路径

### 3. 配置管理
- `config.h/cpp`: 配置类
  - 支持命令行参数解析
  - **新增**: 支持配置文件加载
  - **新增**: 配置文件位于 `config/` 目录

### 4. 线程池
- `threadpool/threadpool.h`: 半同步/半反应堆线程池
  - **升级**: 使用 `std::thread` 替代 pthread
  - **升级**: 使用 lambda 表达式
  - **升级**: RAII 自动管理线程生命周期

### 5. HTTP 连接处理
- `http/http_conn.h/cpp`: HTTP 请求解析和响应生成
  - 状态机解析 HTTP 请求
  - 支持 GET 和 POST 方法
  - **升级**: 使用 `std::map<std::string, std::string>`

### 6. 数据库连接池
- `CGImysql/sql_connection_pool.h/cpp`: MySQL 连接池
  - 单例模式
  - RAII 机制获取和释放连接

### 7. 日志系统
- `log/log.h/cpp`: 同步/异步日志系统
  - 单例模式
  - 支持日志分级
  - 支持按天、按大小分文件

### 8. 定时器
- `timer/lst_timer.h/cpp`: 升序链表定时器
  - 处理非活动连接
  - 定期清理超时连接

### 9. 同步机制
- `lock/locker.h`: 线程同步封装
  - **升级**: `locker` 类封装 `std::mutex`
  - **升级**: `cond` 类封装 `std::condition_variable`
  - `sem` 类封装 POSIX 信号量

## C++17 升级亮点

### 1. 现代化内存管理
- 智能指针替代原始指针
- RAII 原则自动管理资源
- 减少内存泄漏风险

### 2. 标准线程库
- `std::thread` 替代 pthread
- `std::mutex` 和 `std::condition_variable`
- 更好的跨平台支持

### 3. 标准容器
- `std::vector` 替代原始数组
- `std::string` 替代 char*
- 类型安全，自动内存管理

### 4. 文件系统
- `std::filesystem` 处理路径
- 更安全的文件操作

### 5. 配置文件支持
- 支持从文件加载配置
- INI 风格配置格式
- 命令行参数优先级更高

## 编译产物

编译后会生成：
- `server`: 可执行文件
- `*ServerLog`: 日志文件（由 .gitignore 排除）

## 文档说明

### 用户文档
- `README_CPP17.md`: C++17 版本的项目说明
- `docs/QUICKSTART.md`: 快速开始指南
- `docs/UPGRADE_NOTES.md`: 详细的升级说明

### 开发文档
- `docs/COMPLETION_REPORT.md`: 升级完成报告
- 各模块的 `README.md`: 模块功能说明

## 配置文件

- `config/server.conf`: 服务器配置文件
  - INI 格式
  - 支持注释
  - 可配置所有运行参数

## 使用建议

1. **开发阶段**: 使用命令行参数，灵活调试
2. **生产环境**: 使用配置文件，便于管理
3. **测试阶段**: 配置文件 + 命令行参数组合使用

## 相关链接

- [原项目](https://github.com/qinguoyi/TinyWebServer)
- [C++17 版本](https://github.com/Lioooooo123/TinyWebServer-CPP17)
