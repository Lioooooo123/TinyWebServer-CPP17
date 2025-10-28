// Copyright 2025 TinyWebServer
// HTTP connection handler with HTTP/1.1 protocol parsing
// Follows Google C++ Style Guide

#ifndef TINYWEBSERVER_HTTP_HTTP_CONN_H_
#define TINYWEBSERVER_HTTP_HTTP_CONN_H_

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <map>
#include <mysql/mysql.h>
#include <string>
#include <vector>

#include "../CGImysql/sql_connection_pool.h"
#include "../log/log.h"
#include "../timer/lst_timer.h"

namespace tinywebserver {

// HTTP connection class for handling HTTP requests and responses
class HttpConnection {
 public:
  // Constants
  static constexpr int kFileNameLen = 200;
  static constexpr int kReadBufferSize = 2048;
  static constexpr int kWriteBufferSize = 1024;

  // HTTP method enumeration
  enum class Method {
    kGet = 0,
    kPost,
    kHead,
    kPut,
    kDelete,
    kTrace,
    kOptions,
    kConnect,
    kPatch
  };

  // Parse state enumeration
  enum class CheckState {
    kRequestLine = 0,
    kHeader,
    kContent
  };

  // HTTP status code enumeration
  enum class HttpCode {
    kNoRequest,
    kGetRequest,
    kBadRequest,
    kNoResource,
    kForbiddenRequest,
    kFileRequest,
    kInternalError,
    kClosedConnection
  };

  // Line parsing status
  enum class LineStatus {
    kOk = 0,
    kBad,
    kOpen
  };

  HttpConnection() = default;
  ~HttpConnection();

  // Allow move operations
  HttpConnection(HttpConnection&&) = default;
  HttpConnection& operator=(HttpConnection&&) = default;

  // Disable copy operations
  HttpConnection(const HttpConnection&) = delete;
  HttpConnection& operator=(const HttpConnection&) = delete;

  // Initializes the HTTP connection.
  // @param sockfd Socket file descriptor
  // @param addr Client address
  // @param root Document root directory
  // @param trigger_mode Trigger mode (0=LT, 1=ET)
  // @param close_log Log disable flag
  // @param user Database username
  // @param passwd Database password
  // @param sqlname Database name
  void init(int sockfd, const sockaddr_in& addr, char* root, int trigger_mode,
            int close_log, const std::string& user, const std::string& passwd,
            const std::string& sqlname);

  // Closes the connection.
  // @param real_close Whether to actually close the socket
  void close_conn(bool real_close = true);

  // Processes the HTTP request.
  void process();

  // Reads data from socket once (non-blocking).
  // @return true if read successfully, false otherwise
  bool read_once();

  // Writes data to socket.
  // @return true if write successfully, false otherwise
  bool write();

  // Gets client address.
  sockaddr_in* get_address() { return &address_; }

  // Initializes MySQL result set with user data.
  // @param conn_pool Connection pool
  void initmysql_result(ConnectionPool* conn_pool);

  // Public members for timer and state management
  int timer_flag{0};
  int improv{0};
  int m_state{0};  // 0=read, 1=write
  MYSQL* mysql{nullptr};

  // Static members
  static int m_epollfd;
  static int m_user_count;

 private:
  // Internal initialization
  void init();

  // HTTP parsing
  HttpCode ProcessRead();
  bool ProcessWrite(HttpCode ret);

  HttpCode ParseRequestLine(char* text);
  HttpCode ParseHeaders(char* text);
  HttpCode ParseContent(char* text);
  HttpCode DoRequest();

  char* GetLine() { return &read_buf_[start_line_]; }
  LineStatus ParseLine();

  void Unmap();

  // Response building
  bool AddResponse(const char* format, ...);
  bool AddContent(const char* content);
  bool AddStatusLine(int status, const char* title);
  bool AddHeaders(int content_length);
  bool AddContentType();
  bool AddContentLength(int content_length);
  bool AddLinger();
  bool AddBlankLine();

 private:
  int sockfd_{-1};
  sockaddr_in address_{};

  std::vector<char> read_buf_;
  size_t read_idx_{0};
  size_t checked_idx_{0};
  int start_line_{0};

  std::vector<char> write_buf_;
  int write_idx_{0};

  CheckState check_state_{CheckState::kRequestLine};
  Method method_{Method::kGet};

  std::vector<char> real_file_;
  char* url_{nullptr};
  char* version_{nullptr};
  char* host_{nullptr};
  size_t content_length_{0};
  bool linger_{false};

  char* file_address_{nullptr};
  struct stat file_stat_{};
  struct iovec iov_[2]{};
  int iov_count_{0};

  int cgi_{0};
  char* string_{nullptr};
  int bytes_to_send_{0};
  int bytes_have_send_{0};
  char* doc_root_{nullptr};

  std::map<std::string, std::string> users_;
  int trigger_mode_{0};
  int close_log_{0};

  std::vector<char> sql_user_;
  std::vector<char> sql_passwd_;
  std::vector<char> sql_name_;
};

// Utility functions
int SetNonBlocking(int fd);
void AddFd(int epollfd, int fd, bool one_shot, int trigger_mode);
void RemoveFd(int epollfd, int fd);
void ModifyFd(int epollfd, int fd, int ev, int trigger_mode);

// Helper function for timer callback
void DecrementHttpUserCount();

}  // namespace tinywebserver

#endif  // TINYWEBSERVER_HTTP_HTTP_CONN_H_

