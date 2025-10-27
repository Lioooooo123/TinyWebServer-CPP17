// Copyright 2025 TinyWebServer
// WebServer 类的实现

#include "webserver.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>

#include <cassert>
#include <cstring>
#include <filesystem>

namespace tinywebserver {

WebServer::WebServer()
    : port_(0),
      root_dir_(),
      log_write_mode_(0),
      close_log_(0),
      actor_model_(0),
      pipe_fd_{-1, -1},
      epoll_fd_(-1),
      users_(kMaxFd),
      conn_pool_(nullptr),
      db_user_(),
      db_password_(),
      db_name_(),
      sql_connection_num_(0),
      thread_pool_(nullptr),
      thread_num_(0),
      listen_fd_(-1),
      opt_linger_(0),
      trigger_mode_(0),
      listen_trigger_mode_(0),
      conn_trigger_mode_(0),
      users_timer_(kMaxFd) {
  // 设置根目录路径
  root_dir_ = (std::filesystem::current_path() / "root").string();
}

WebServer::~WebServer() {
  if (epoll_fd_ != -1) {
    close(epoll_fd_);
    epoll_fd_ = -1;
  }
  if (listen_fd_ != -1) {
    close(listen_fd_);
    listen_fd_ = -1;
  }
  if (pipe_fd_[1] != -1) {
    close(pipe_fd_[1]);
    pipe_fd_[1] = -1;
  }
  if (pipe_fd_[0] != -1) {
    close(pipe_fd_[0]);
    pipe_fd_[0] = -1;
  }
}

void WebServer::Init(int port, const std::string& user,
                     const std::string& password,
                     const std::string& database_name, int log_write_mode,
                     int opt_linger, int trigger_mode, int sql_num,
                     int thread_num, int close_log, int actor_model) {
  port_ = port;
  db_user_ = user;
  db_password_ = password;
  db_name_ = database_name;
  sql_connection_num_ = sql_num;
  thread_num_ = thread_num;
  log_write_mode_ = log_write_mode;
  opt_linger_ = opt_linger;
  trigger_mode_ = trigger_mode;
  close_log_ = close_log;
  actor_model_ = actor_model;
}

void WebServer::SetTriggerMode() {
  // LT + LT
  if (trigger_mode_ == 0) {
    listen_trigger_mode_ = 0;
    conn_trigger_mode_ = 0;
  }
  // LT + ET
  else if (trigger_mode_ == 1) {
    listen_trigger_mode_ = 0;
    conn_trigger_mode_ = 1;
  }
  // ET + LT
  else if (trigger_mode_ == 2) {
    listen_trigger_mode_ = 1;
    conn_trigger_mode_ = 0;
  }
  // ET + ET
  else if (trigger_mode_ == 3) {
    listen_trigger_mode_ = 1;
    conn_trigger_mode_ = 1;
  }
}

void WebServer::InitLog() {
  if (close_log_ == 0) {
    // 初始化日志系统
    if (log_write_mode_ == 1) {
      Logger::GetInstance()->Init("./ServerLog", close_log_, 2000, 800000, 800);
    } else {
      Logger::GetInstance()->Init("./ServerLog", close_log_, 2000, 800000, 0);
    }
  }
}

void WebServer::InitSqlPool() {
  // 初始化数据库连接池
  conn_pool_ = ConnectionPool::GetInstance();
  conn_pool_->Init("localhost", db_user_, db_password_, db_name_, 3306,
                   sql_connection_num_, close_log_);

  // 初始化数据库用户表
  if (!users_.empty()) {
    users_.front().initmysql_result(conn_pool_);
  }
}

void WebServer::InitThreadPool() {
  // 初始化线程池
  thread_pool_ =
      std::make_unique<ThreadPool<HttpConnection>>(actor_model_, conn_pool_, thread_num_);
}

void WebServer::StartListen() {
  // 创建监听套接字
  listen_fd_ = socket(PF_INET, SOCK_STREAM, 0);
  assert(listen_fd_ >= 0);

  // 配置 SO_LINGER 选项
  if (opt_linger_ == 0) {
    struct linger tmp = {0, 1};
    setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
  } else if (opt_linger_ == 1) {
    struct linger tmp = {1, 1};
    setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
  }

  // 绑定套接字
  int ret = 0;
  struct sockaddr_in address;
  std::memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(port_);

  int flag = 1;
  setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
  ret = bind(listen_fd_, reinterpret_cast<struct sockaddr*>(&address),
             sizeof(address));
  assert(ret >= 0);
  ret = listen(listen_fd_, 5);
  assert(ret >= 0);

  timer_utils_.Init(kTimeSlot);

  // 创建 epoll 实例
  epoll_fd_ = epoll_create(5);
  assert(epoll_fd_ != -1);

  timer_utils_.AddFd(epoll_fd_, listen_fd_, false, listen_trigger_mode_);
  HttpConnection::m_epollfd = epoll_fd_;

  // 创建信号管道
  ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipe_fd_);
  assert(ret != -1);
  timer_utils_.SetNonBlocking(pipe_fd_[1]);
  timer_utils_.AddFd(epoll_fd_, pipe_fd_[0], false, 0);

  // 注册信号处理器
  timer_utils_.AddSignal(SIGPIPE, SIG_IGN);
  timer_utils_.AddSignal(SIGALRM, TimerUtils::SignalHandler, false);
  timer_utils_.AddSignal(SIGTERM, TimerUtils::SignalHandler, false);

  alarm(kTimeSlot);

  // 设置用于信号处理的静态成员
  TimerUtils::pipe_fd_ = pipe_fd_;
  TimerUtils::epoll_fd_ = epoll_fd_;
}

void WebServer::AddTimer(int connfd, const sockaddr_in& client_address) {
  users_[connfd].init(connfd, client_address,
                      const_cast<char*>(root_dir_.c_str()), conn_trigger_mode_,
                      close_log_, db_user_, db_password_, db_name_);

  // 初始化客户端数据并创建定时器
  users_timer_[connfd].address = client_address;
  users_timer_[connfd].sockfd = connfd;

  Timer* timer = new Timer;
  timer->user_data_ = &users_timer_[connfd];
  timer->callback_ = TimerCallback;

  auto now = std::chrono::steady_clock::now();
  timer->expire_time_ = now + std::chrono::seconds(3 * kTimeSlot);

  users_timer_[connfd].timer = timer;
  timer_utils_.timer_list_.AddTimer(timer);
}

void WebServer::AdjustTimer(Timer* timer) {
  auto now = std::chrono::steady_clock::now();
  timer->expire_time_ = now + std::chrono::seconds(3 * kTimeSlot);
  timer_utils_.timer_list_.AdjustTimer(timer);

  LOG_INFO("%s", "adjust timer once");
}

void WebServer::HandleTimer(Timer* timer, int sockfd) {
  if (timer && timer->callback_) {
    timer->callback_(&users_timer_[sockfd]);
  }
  if (timer) {
    timer_utils_.timer_list_.DeleteTimer(timer);
  }

  LOG_INFO("close fd %d", users_timer_[sockfd].sockfd);
  users_timer_[sockfd].timer = nullptr;
}

bool WebServer::HandleClientData() {
  struct sockaddr_in client_address;
  socklen_t client_addrlength = sizeof(client_address);

  if (listen_trigger_mode_ == 0) {
    // LT 模式
    int connfd = accept(listen_fd_, reinterpret_cast<struct sockaddr*>(&client_address),
                        &client_addrlength);
    if (connfd < 0) {
      LOG_ERROR("%s:errno is:%d", "accept error", errno);
      return false;
    }
    if (HttpConnection::m_user_count >= kMaxFd) {
      timer_utils_.ShowError(connfd, "Internal server busy");
      LOG_ERROR("%s", "Internal server busy");
      return false;
    }
    AddTimer(connfd, client_address);
  } else {
    // ET 模式
    while (true) {
      int connfd = accept(listen_fd_, reinterpret_cast<struct sockaddr*>(&client_address),
                          &client_addrlength);
      if (connfd < 0) {
        LOG_ERROR("%s:errno is:%d", "accept error", errno);
        break;
      }
      if (HttpConnection::m_user_count >= kMaxFd) {
        timer_utils_.ShowError(connfd, "Internal server busy");
        LOG_ERROR("%s", "Internal server busy");
        break;
      }
      AddTimer(connfd, client_address);
    }
    return false;
  }
  return true;
}

bool WebServer::HandleSignal(bool& timeout, bool& stop_server) {
  int ret = 0;
  int sig;
  char signals[1024];
  ret = recv(pipe_fd_[0], signals, sizeof(signals), 0);

  if (ret == -1) {
    return false;
  } else if (ret == 0) {
    return false;
  } else {
    for (int i = 0; i < ret; ++i) {
      switch (signals[i]) {
        case SIGALRM: {
          timeout = true;
          break;
        }
        case SIGTERM: {
          stop_server = true;
          break;
        }
      }
    }
  }
  return true;
}

void WebServer::HandleRead(int sockfd) {
  Timer* timer = users_timer_[sockfd].timer;

  // Reactor 模型
  if (actor_model_ == 1) {
    if (timer) {
      AdjustTimer(timer);
    }

    // 将读事件添加到请求队列
    thread_pool_->Append(std::shared_ptr<HttpConnection>(&users_[sockfd], [](HttpConnection*){}), 0);

    while (true) {
      if (users_[sockfd].improv == 1) {
        if (users_[sockfd].timer_flag == 1) {
          HandleTimer(timer, sockfd);
          users_[sockfd].timer_flag = 0;
        }
        users_[sockfd].improv = 0;
        break;
      }
    }
  } else {
    // Proactor 模型
    if (users_[sockfd].read_once()) {
      LOG_INFO("deal with the client(%s)",
               inet_ntoa(users_[sockfd].get_address()->sin_addr));

      // 将读事件添加到请求队列
      thread_pool_->AppendProactor(std::shared_ptr<HttpConnection>(&users_[sockfd], [](HttpConnection*){}));

      if (timer) {
        AdjustTimer(timer);
      }
    } else {
      HandleTimer(timer, sockfd);
    }
  }
}

void WebServer::HandleWrite(int sockfd) {
  Timer* timer = users_timer_[sockfd].timer;

  // Reactor 模型
  if (actor_model_ == 1) {
    if (timer) {
      AdjustTimer(timer);
    }

    thread_pool_->Append(std::shared_ptr<HttpConnection>(&users_[sockfd], [](HttpConnection*){}), 1);

    while (true) {
      if (users_[sockfd].improv == 1) {
        if (users_[sockfd].timer_flag == 1) {
          HandleTimer(timer, sockfd);
          users_[sockfd].timer_flag = 0;
        }
        users_[sockfd].improv = 0;
        break;
      }
    }
  } else {
    // Proactor 模型
    if (users_[sockfd].write()) {
      LOG_INFO("send data to the client(%s)",
               inet_ntoa(users_[sockfd].get_address()->sin_addr));

      if (timer) {
        AdjustTimer(timer);
      }
    } else {
      HandleTimer(timer, sockfd);
    }
  }
}

void WebServer::EventLoop() {
  bool timeout = false;
  bool stop_server = false;

  while (!stop_server) {
    int number = epoll_wait(epoll_fd_, events_, kMaxEventNumber, -1);
    if (number < 0 && errno != EINTR) {
      LOG_ERROR("%s", "epoll failure");
      break;
    }

    for (int i = 0; i < number; i++) {
      int sockfd = events_[i].data.fd;

      // 处理新的客户端连接
      if (sockfd == listen_fd_) {
        bool flag = HandleClientData();
        if (flag == false) {
          continue;
        }
      } else if (events_[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        // 服务器关闭连接，移除定时器
        Timer* timer = users_timer_[sockfd].timer;
        HandleTimer(timer, sockfd);
      }
      // 处理信号
      else if ((sockfd == pipe_fd_[0]) && (events_[i].events & EPOLLIN)) {
        bool flag = HandleSignal(timeout, stop_server);
        if (flag == false) {
          LOG_ERROR("%s", "dealclientdata failure");
        }
      }
      // 处理客户端数据
      else if (events_[i].events & EPOLLIN) {
        HandleRead(sockfd);
      } else if (events_[i].events & EPOLLOUT) {
        HandleWrite(sockfd);
      }
    }

    if (timeout) {
      timer_utils_.HandleTimer();
      LOG_INFO("%s", "timer tick");
      timeout = false;
    }
  }
}

}  // namespace tinywebserver

