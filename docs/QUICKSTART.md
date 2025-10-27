# 快速开始指南

## 编译项目

```bash
# 清理旧的编译文件
make clean

# 编译项目
make server
```

## 运行方式

### 1. 使用配置文件运行（推荐）

```bash
./server -f config/server.conf
```

### 2. 使用命令行参数运行

```bash
./server -p 9006 -t 8 -s 8 -l 0 -m 0
```

### 3. 混合模式（配置文件 + 命令行参数）

命令行参数会覆盖配置文件中的设置：

```bash
./server -f config/server.conf -p 8080 -t 16
```

## 命令行参数说明

| 参数 | 说明 | 默认值 | 可选值 |
|------|------|--------|--------|
| `-p` | 端口号 | 9006 | 1-65535 |
| `-l` | 日志写入方式 | 0 | 0=同步, 1=异步 |
| `-m` | 触发模式 | 0 | 0=LT+LT, 1=LT+ET, 2=ET+LT, 3=ET+ET |
| `-o` | 优雅关闭连接 | 0 | 0=不使用, 1=使用 |
| `-s` | 数据库连接池数量 | 8 | 1-100 |
| `-t` | 线程池线程数量 | 8 | 1-100 |
| `-c` | 关闭日志 | 0 | 0=开启, 1=关闭 |
| `-a` | 并发模型 | 0 | 0=Proactor, 1=Reactor |
| `-f` | 配置文件路径 | 无 | 文件路径 |

## 配置文件格式

配置文件位于 `config/server.conf`，格式如下：

```ini
# 注释以 # 开头
PORT=9006
LOGWrite=0
TRIGMode=0
OPT_LINGER=0
sql_num=8
thread_num=8
close_log=0
actor_model=0
```

## 数据库配置

### 1. 创建数据库

```sql
CREATE DATABASE yourdb;
```

### 2. 创建用户表

```sql
USE yourdb;
CREATE TABLE user(
    username CHAR(50) NULL,
    passwd CHAR(50) NULL
) ENGINE=InnoDB;
```

### 3. 插入测试数据

```sql
INSERT INTO user(username, passwd) VALUES('testuser', 'testpass');
```

### 4. 修改代码中的数据库配置

编辑 `main.cpp`：

```cpp
std::string user = "root";          // 数据库用户名
std::string passwd = "root";        // 数据库密码
std::string databasename = "yourdb"; // 数据库名
```

## 访问服务器

启动服务器后，在浏览器中访问：

```
http://localhost:9006
```

或者使用服务器IP地址：

```
http://your_server_ip:9006
```

## 常见问题

### 1. 编译错误

确保安装了必要的依赖：

```bash
sudo apt-get install g++ make libmysqlclient-dev
```

### 2. 数据库连接失败

检查：
- MySQL 服务是否运行
- 数据库名、用户名、密码是否正确
- 数据库用户是否有相应权限

### 3. 端口被占用

使用其他端口：

```bash
./server -p 8080
```

或者查看并关闭占用端口的进程：

```bash
netstat -tlnp | grep 9006
kill -9 <pid>
```

## 性能测试

使用 webbench 进行压力测试：

```bash
cd test_pressure/webbench-1.5
make
./webbench -c 10500 -t 5 http://localhost:9006/
```

参数说明：
- `-c`: 并发客户端数量
- `-t`: 测试持续时间（秒）

## 日志查看

日志文件位于项目根目录，格式为 `YYYY_MM_DD_ServerLog`：

```bash
# 查看最新日志
tail -f *ServerLog

# 搜索错误日志
grep "ERROR" *ServerLog
```

## 更多信息

- [升级说明](../docs/UPGRADE_NOTES.md)
- [完成报告](../docs/COMPLETION_REPORT.md)
- [原项目文档](https://github.com/qinguoyi/TinyWebServer)
