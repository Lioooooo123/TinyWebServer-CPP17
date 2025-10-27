// Copyright 2025 TinyWebServer
// 基于 RAII 的 MySQL 连接池及连接管理
// 遵循 Google C++ 编码规范

#ifndef TINYWEBSERVER_CGIMYSQL_SQL_CONNECTION_POOL_H_
#define TINYWEBSERVER_CGIMYSQL_SQL_CONNECTION_POOL_H_

#include <mysql/mysql.h>

#include <atomic>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <string>

#include "../log/log.h"

namespace tinywebserver {

// 使用单例模式的 MySQL 连接池。
// 提供线程安全的数据库连接访问。
class ConnectionPool {
 public:
  // 获取 ConnectionPool 的单例实例。
  static ConnectionPool* GetInstance();

  // 禁用拷贝和移动操作
  ConnectionPool(const ConnectionPool&) = delete;
  ConnectionPool& operator=(const ConnectionPool&) = delete;
  ConnectionPool(ConnectionPool&&) = delete;
  ConnectionPool& operator=(ConnectionPool&&) = delete;

  // 使用数据库配置初始化连接池。
  // @param url 数据库主机 URL
  // @param user 数据库用户名
  // @param password 数据库密码
  // @param db_name 数据库名
  // @param port 数据库端口
  // @param max_conn 连接池中的最大连接数
  // @param close_log 日志禁用标志 (0=启用, 1=禁用)
  void Init(const std::string& url, const std::string& user,
            const std::string& password, const std::string& db_name, int port,
            int max_conn, int close_log);

  // 从连接池获取一个数据库连接。
  // 如果没有可用连接则阻塞。
  // @return MySQL 连接指针，如果连接池被销毁则返回 nullptr
  MYSQL* GetConnection();

  // 将连接释放回连接池。
  // @param conn 要释放的连接
  // @return 如果成功返回 true，如果 conn 为 nullptr 返回 false
  bool ReleaseConnection(MYSQL* conn);

  // 获取空闲连接数量。
  int GetFreeConnCount() const { return free_conn_count_; }

  // 获取当前正在使用的连接数量。
  int GetCurConnCount() const { return cur_conn_count_; }

  // 销毁连接池中的所有连接。
  void DestroyPool();

 private:
  ConnectionPool();
  ~ConnectionPool();

  int max_conn_;                      // 最大连接数
  std::atomic<int> cur_conn_count_;   // 当前正在使用的连接数
  std::atomic<int> free_conn_count_;  // 当前空闲连接数

  mutable std::mutex mutex_;          // 线程安全互斥锁
  std::condition_variable cond_;      // 阻塞用的条件变量
  std::list<MYSQL*> conn_list_;       // 连接列表

  std::atomic<bool> is_destroyed_;  // 销毁标志

 public:
  std::string url_;         // 数据库主机
  std::string port_;        // 数据库端口
  std::string user_;        // 数据库用户名
  std::string password_;    // 数据库密码
  std::string db_name_;     // 数据库名
  int close_log_;           // 日志禁用标志
};

// 用于自动连接管理的 RAII 包装器。
// 在构造函数中获取连接，在析构函数中释放连接。
class ConnectionRAII {
 public:
  // 构造 ConnectionRAII 并从连接池获取一个连接。
  // @param conn 获取的连接的输出参数
  // @param pool 从中获取连接的连接池
  ConnectionRAII(MYSQL** conn, ConnectionPool* pool);

  // 析构函数将连接释放回连接池。
  ~ConnectionRAII();

  // 禁用拷贝和移动操作
  ConnectionRAII(const ConnectionRAII&) = delete;
  ConnectionRAII& operator=(const ConnectionRAII&) = delete;
  ConnectionRAII(ConnectionRAII&&) = delete;
  ConnectionRAII& operator=(ConnectionRAII&&) = delete;

 private:
  MYSQL* conn_raii_;              // 由此 RAII 对象管理的连接
  ConnectionPool* pool_raii_;     // 从中获取连接的连接池
};

}  // namespace tinywebserver

#endif  // TINYWEBSERVER_CGIMYSQL_SQL_CONNECTION_POOL_H_

