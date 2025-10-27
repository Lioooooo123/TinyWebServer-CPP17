// Copyright 2025 TinyWebServer
// Main web server class handling connections, events, and request routing
// Follows Google C++ Style Guide

#ifndef TINYWEBSERVER_WEBSERVER_H_
#define TINYWEBSERVER_WEBSERVER_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "./CGImysql/sql_connection_pool.h"
#include "./http/http_conn.h"
#include "./log/log.h"
#include "./threadpool/threadpool.h"
#include "./timer/lst_timer.h"

namespace tinywebserver {

// Constants
constexpr int kMaxFd = 65536;           // Maximum file descriptors
constexpr int kMaxEventNumber = 10000;  // Maximum epoll events
constexpr int kTimeSlot = 5;            // Minimum timeout unit (seconds)

// Main web server class managing connections, events, and request processing.
class WebServer {
 public:
  WebServer();
  ~WebServer();

  // Disable copy and move operations
  WebServer(const WebServer&) = delete;
  WebServer& operator=(const WebServer&) = delete;
  WebServer(WebServer&&) = delete;
  WebServer& operator=(WebServer&&) = delete;

  // Initializes the server with configuration parameters.
  // @param port Server port number
  // @param user Database username
  // @param password Database password
  // @param database_name Database name
  // @param log_write_mode Log mode (0=sync, 1=async)
  // @param opt_linger SO_LINGER option
  // @param trigger_mode Trigger mode combination
  // @param sql_num Database connection pool size
  // @param thread_num Thread pool size
  // @param close_log Log disable flag
  // @param actor_model Concurrency model (0=Proactor, 1=Reactor)
  void Init(int port, const std::string& user, const std::string& password,
            const std::string& database_name, int log_write_mode,
            int opt_linger, int trigger_mode, int sql_num, int thread_num,
            int close_log, int actor_model);

  // Initializes thread pool
  void InitThreadPool();

  // Initializes database connection pool
  void InitSqlPool();

  // Initializes logging system
  void InitLog();

  // Sets trigger mode for listen and connection sockets
  void SetTriggerMode();

  // Starts listening for connections
  void StartListen();

  // Main event loop for handling connections and events
  void EventLoop();

  // Adds a timer for the specified connection.
  // @param connfd Connection file descriptor
  // @param client_address Client socket address
  void AddTimer(int connfd, const sockaddr_in& client_address);

  // Adjusts timer expiration time.
  // @param timer Timer to adjust
  void AdjustTimer(Timer* timer);

  // Handles timer expiration and closes connection.
  // @param timer Timer that expired
  // @param sockfd Socket file descriptor
  void HandleTimer(Timer* timer, int sockfd);

  // Handles new client connections.
  // @return true if successful, false otherwise
  bool HandleClientData();

  // Handles signals received via pipe.
  // @param timeout Output parameter set to true if SIGALRM received
  // @param stop_server Output parameter set to true if SIGTERM received
  // @return true if successful, false otherwise
  bool HandleSignal(bool& timeout, bool& stop_server);

  // Handles read event on socket.
  // @param sockfd Socket file descriptor
  void HandleRead(int sockfd);

  // Handles write event on socket.
  // @param sockfd Socket file descriptor
  void HandleWrite(int sockfd);

 private:
  // Basic configuration
  int port_;
  std::string root_dir_;
  int log_write_mode_;
  int close_log_;
  int actor_model_;

  // File descriptors
  int pipe_fd_[2];
  int epoll_fd_;
  std::vector<HttpConnection> users_;

  // Database connection pool
  ConnectionPool* conn_pool_;
  std::string db_user_;
  std::string db_password_;
  std::string db_name_;
  int sql_connection_num_;

  // Thread pool
  std::unique_ptr<ThreadPool<HttpConnection>> thread_pool_;
  int thread_num_;

  // Epoll events
  epoll_event events_[kMaxEventNumber];

  // Listen socket
  int listen_fd_;
  int opt_linger_;
  int trigger_mode_;
  int listen_trigger_mode_;
  int conn_trigger_mode_;

  // Timer management
  std::vector<ClientData> users_timer_;
  TimerUtils timer_utils_;
};

}  // namespace tinywebserver

#endif  // TINYWEBSERVER_WEBSERVER_H_

