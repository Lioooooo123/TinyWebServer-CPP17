// Copyright 2025 TinyWebServer
// MySQL 连接池的实现

#include "sql_connection_pool.h"

#include <cstdlib>
#include <cstring>

namespace tinywebserver {

// ConnectionPool 实现

ConnectionPool::ConnectionPool()
    : max_conn_(0),
      cur_conn_count_(0),
      free_conn_count_(0),
      is_destroyed_(false),
      close_log_(0) {}

ConnectionPool::~ConnectionPool() {
  DestroyPool();
}

ConnectionPool* ConnectionPool::GetInstance() {
  static ConnectionPool conn_pool;
  return &conn_pool;
}

void ConnectionPool::Init(const std::string& url, const std::string& user,
                          const std::string& password,
                          const std::string& db_name, int port, int max_conn,
                          int close_log) {
  url_ = url;
  port_ = std::to_string(port);
  user_ = user;
  password_ = password;
  db_name_ = db_name;
  close_log_ = close_log;
  max_conn_ = max_conn;

  // 创建 max_conn 个数据库连接
  for (int i = 0; i < max_conn; ++i) {
    MYSQL* conn = nullptr;
    conn = mysql_init(conn);

    if (conn == nullptr) {
      LOG_ERROR("MySQL Init Error");
      std::exit(1);
    }

    conn = mysql_real_connect(conn, url.c_str(), user.c_str(),
                               password.c_str(), db_name.c_str(), port,
                               nullptr, 0);

    if (conn == nullptr) {
      LOG_ERROR("MySQL Connect Error: %s", mysql_error(conn));
      std::exit(1);
    }

    conn_list_.push_back(conn);
    ++free_conn_count_;
  }

  cur_conn_count_ = 0;

  if (close_log_ == 0) {
    LOG_INFO("Connection pool init success! MaxConn: %d", max_conn);
  }
}

MYSQL* ConnectionPool::GetConnection() {
  MYSQL* conn = nullptr;

  if (is_destroyed_) {
    return nullptr;
  }

  std::unique_lock<std::mutex> lock(mutex_);

  // 等待可用连接
  cond_.wait(lock, [this] {
    return !conn_list_.empty() || is_destroyed_;
  });

  if (is_destroyed_) {
    return nullptr;
  }

  conn = conn_list_.front();
  conn_list_.pop_front();

  --free_conn_count_;
  ++cur_conn_count_;

  return conn;
}

bool ConnectionPool::ReleaseConnection(MYSQL* conn) {
  if (conn == nullptr) {
    return false;
  }

  {
    std::lock_guard<std::mutex> lock(mutex_);
    conn_list_.push_back(conn);
    ++free_conn_count_;
    --cur_conn_count_;
  }

  // 通知等待中的线程有可用连接
  cond_.notify_one();
  return true;
}

void ConnectionPool::DestroyPool() {
  if (is_destroyed_) {
    return;
  }

  is_destroyed_ = true;

  {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!conn_list_.empty()) {
      for (auto it = conn_list_.begin(); it != conn_list_.end(); ++it) {
        MYSQL* conn = *it;
        mysql_close(conn);
      }

      cur_conn_count_ = 0;
      free_conn_count_ = 0;
      conn_list_.clear();
    }
  }

  // 通知所有等待中的线程连接池已被销毁
  cond_.notify_all();

  if (close_log_ == 0) {
    LOG_INFO("Connection pool destroyed!");
  }
}

// ConnectionRAII 实现

ConnectionRAII::ConnectionRAII(MYSQL** conn, ConnectionPool* pool)
    : conn_raii_(nullptr), pool_raii_(pool) {
  *conn = pool->GetConnection();
  conn_raii_ = *conn;
}

ConnectionRAII::~ConnectionRAII() {
  if (conn_raii_ != nullptr && pool_raii_ != nullptr) {
    pool_raii_->ReleaseConnection(conn_raii_);
  }
}

}  // namespace tinywebserver

