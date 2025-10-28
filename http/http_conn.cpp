// Copyright 2025 TinyWebServer
// Implementation of HTTP connection handler

#include "http_conn.h"

#include <mysql/mysql.h>
#include <cstdarg>
#include <cstring>
#include <mutex>

namespace tinywebserver {

// HTTP response status information
const char* kOk200Title = "OK";
const char* kError400Title = "Bad Request";
const char* kError400Form =
    "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* kError403Title = "Forbidden";
const char* kError403Form =
    "You do not have permission to get file from this server.\n";
const char* kError404Title = "Not Found";
const char* kError404Form =
    "The requested file was not found on this server.\n";
const char* kError500Title = "Internal Error";
const char* kError500Form =
    "There was an unusual problem serving the request file.\n";

std::mutex users_mutex;
std::map<std::string, std::string> users;

void HttpConnection::initmysql_result(ConnectionPool* connPool) {
  // 先从连接池中取一个连接
  MYSQL* mysql = nullptr;
  ConnectionRAII mysqlcon(&mysql, connPool);

  if (mysql == nullptr) {
    LOG_ERROR("%s", "MySQL connection retrieval failed");
    return;
  }

  // 在 user 表中检索 username，passwd 数据
  if (mysql_query(mysql, "SELECT username,passwd FROM user")) {
    LOG_ERROR("SELECT error: %s", mysql_error(mysql));
    return;
  }

  // 从表中检索完整的结果集
  MYSQL_RES* result = mysql_store_result(mysql);
  if (result == nullptr) {
    if (mysql_field_count(mysql) != 0) {
      LOG_ERROR("mysql_store_result error: %s", mysql_error(mysql));
    }
    return;
  }

  std::lock_guard<std::mutex> lock(users_mutex);
  users.clear();

  // 从结果集中获取下一行，将对应的用户名和密码，存入 map 中
  while (MYSQL_ROW row = mysql_fetch_row(result)) {
    if (row[0] == nullptr || row[1] == nullptr) {
      continue;
    }
    users[std::string(row[0])] = std::string(row[1]);
  }

  mysql_free_result(result);
}

// 对文件描述符设置非阻塞
int SetNonBlocking(int fd) {
  int old_option = fcntl(fd, F_GETFL);
  int new_option = old_option | O_NONBLOCK;
  fcntl(fd, F_SETFL, new_option);
  return old_option;
}

// 将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void AddFd(int epollfd, int fd, bool one_shot, int trigger_mode) {
  epoll_event event;
  event.data.fd = fd;

  if (1 == trigger_mode)
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
  else
    event.events = EPOLLIN | EPOLLRDHUP;

  if (one_shot) event.events |= EPOLLONESHOT;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
  SetNonBlocking(fd);
}

// 从内核时间表删除描述符
void RemoveFd(int epollfd, int fd) {
  epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
  close(fd);
}

// 将事件重置为EPOLLONESHOT
void ModifyFd(int epollfd, int fd, int ev, int trigger_mode) {
  epoll_event event;
  event.data.fd = fd;

  if (1 == trigger_mode)
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
  else
    event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

  epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

int HttpConnection::m_user_count = 0;
int HttpConnection::m_epollfd = -1;

HttpConnection::~HttpConnection() {
  // Destructor implementation
}

// 关闭连接，关闭一个连接，客户总量减一
void HttpConnection::close_conn(bool real_close) {
  if (real_close && (sockfd_ != -1)) {
    printf("close %d\n", sockfd_);
    RemoveFd(m_epollfd, sockfd_);
    sockfd_ = -1;
    m_user_count--;
  }
}

// 初始化连接,外部调用初始化套接字地址
void HttpConnection::init(int sockfd, const sockaddr_in& addr, char* root,
                          int trigger_mode, int close_log,
                          const std::string& user, const std::string& passwd,
                          const std::string& sqlname) {
  sockfd_ = sockfd;
  address_ = addr;

  AddFd(m_epollfd, sockfd, true, trigger_mode);
  m_user_count++;

  // 当浏览器出现连接重置时，可能是网站根目录出错或http响应格式出错或者访问的文件中内容完全为空
  doc_root_ = root;
  trigger_mode_ = trigger_mode;
  close_log_ = close_log;

  sql_user_.assign(user.begin(), user.end());
  sql_user_.push_back('\0');
  sql_passwd_.assign(passwd.begin(), passwd.end());
  sql_passwd_.push_back('\0');
  sql_name_.assign(sqlname.begin(), sqlname.end());
  sql_name_.push_back('\0');

  init();
}

// 初始化新接受的连接
// check_state默认为分析请求行状态
void HttpConnection::init() {
  mysql = NULL;
  bytes_to_send_ = 0;
  bytes_have_send_ = 0;
  check_state_ = CheckState::kRequestLine;
  linger_ = false;
  method_ = Method::kGet;
  url_ = 0;
  version_ = 0;
  content_length_ = 0;
  host_ = 0;
  start_line_ = 0;
  checked_idx_ = 0;
  read_idx_ = 0;
  write_idx_ = 0;
  cgi_ = 0;
  m_state = 0;
  timer_flag = 0;
  improv = 0;

  read_buf_.resize(kReadBufferSize);
  std::memset(&read_buf_[0], '\0', kReadBufferSize);
  write_buf_.resize(kWriteBufferSize);
  std::memset(&write_buf_[0], '\0', kWriteBufferSize);
  real_file_.resize(kFileNameLen);
  std::memset(&real_file_[0], '\0', kFileNameLen);
}

// 从状态机，用于分析出一行内容
// 返回值为行的读取状态，有LineStatus::kOk,LineStatus::kBad,LineStatus::kOpen
HttpConnection::LineStatus HttpConnection::ParseLine() {
  char temp;
  for (; checked_idx_ < read_idx_; ++checked_idx_) {
    temp = read_buf_[checked_idx_];
    if (temp == '\r') {
      if ((checked_idx_ + 1) == read_idx_)
        return LineStatus::kOpen;
      else if (read_buf_[checked_idx_ + 1] == '\n') {
        read_buf_[checked_idx_++] = '\0';
        read_buf_[checked_idx_++] = '\0';
        return LineStatus::kOk;
      }
      return LineStatus::kBad;
    } else if (temp == '\n') {
      if (checked_idx_ > 1 && read_buf_[checked_idx_ - 1] == '\r') {
        read_buf_[checked_idx_ - 1] = '\0';
        read_buf_[checked_idx_++] = '\0';
        return LineStatus::kOk;
      }
      return LineStatus::kBad;
    }
  }
  return LineStatus::kOpen;
}

// 循环读取客户数据，直到无数据可读或对方关闭连接
// 非阻塞ET工作模式下，需要一次性将数据读完
bool HttpConnection::read_once() {
  if (read_idx_ >= kReadBufferSize) {
    return false;
  }
  int bytes_read = 0;

  // LT读取数据
  if (0 == trigger_mode_) {
    bytes_read = recv(sockfd_, &read_buf_[read_idx_],
                      kReadBufferSize - read_idx_, 0);
    read_idx_ += bytes_read;

    if (bytes_read <= 0) {
      return false;
    }

    return true;
  }
  // ET读数据
  else {
    while (true) {
      bytes_read = recv(sockfd_, &read_buf_[read_idx_],
                        kReadBufferSize - read_idx_, 0);
      if (bytes_read == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
        return false;
      } else if (bytes_read == 0) {
        return false;
      }
      read_idx_ += bytes_read;
    }
    return true;
  }
}

// 解析http请求行，获得请求方法，目标url及http版本号
HttpConnection::HttpCode HttpConnection::ParseRequestLine(char* text) {
  url_ = strpbrk(text, " \t");
  if (!url_) {
    return HttpCode::kBadRequest;
  }
  *url_++ = '\0';
  char* method = text;
  if (strcasecmp(method, "GET") == 0)
    method_ = Method::kGet;
  else if (strcasecmp(method, "POST") == 0) {
    method_ = Method::kPost;
    cgi_ = 1;
  } else
    return HttpCode::kBadRequest;
  url_ += strspn(url_, " \t");
  version_ = strpbrk(url_, " \t");
  if (!version_) return HttpCode::kBadRequest;
  *version_++ = '\0';
  version_ += strspn(version_, " \t");
  if (strcasecmp(version_, "HTTP/1.1") != 0) return HttpCode::kBadRequest;
  if (strncasecmp(url_, "http://", 7) == 0) {
    url_ += 7;
    url_ = strchr(url_, '/');
  }

  if (strncasecmp(url_, "https://", 8) == 0) {
    url_ += 8;
    url_ = strchr(url_, '/');
  }

  if (!url_ || url_[0] != '/') return HttpCode::kBadRequest;
  // 当url为/时，显示判断界面
  if (strlen(url_) == 1) strcat(url_, "judge.html");
  check_state_ = CheckState::kHeader;
  return HttpCode::kNoRequest;
}

// 解析http请求的一个头部信息
HttpConnection::HttpCode HttpConnection::ParseHeaders(char* text) {
  if (text[0] == '\0') {
    if (content_length_ != 0) {
      check_state_ = CheckState::kContent;
      return HttpCode::kNoRequest;
    }
    return HttpCode::kGetRequest;
  } else if (strncasecmp(text, "Connection:", 11) == 0) {
    text += 11;
    text += strspn(text, " \t");
    if (strcasecmp(text, "keep-alive") == 0) {
      linger_ = true;
    }
  } else if (strncasecmp(text, "Content-length:", 15) == 0) {
    text += 15;
    text += strspn(text, " \t");
    content_length_ = atol(text);
  } else if (strncasecmp(text, "Host:", 5) == 0) {
    text += 5;
    text += strspn(text, " \t");
    host_ = text;
  } else {
    LOG_INFO("oop!unknow header: %s", text);
  }
  return HttpCode::kNoRequest;
}

// 判断http请求是否被完整读入
HttpConnection::HttpCode HttpConnection::ParseContent(char* text) {
  if (read_idx_ >= (content_length_ + checked_idx_)) {
    text[content_length_] = '\0';
    // POST请求中最后为输入的用户名和密码
    string_ = text;
    return HttpCode::kGetRequest;
  }
  return HttpCode::kNoRequest;
}

HttpConnection::HttpCode HttpConnection::ProcessRead() {
  LineStatus line_status = LineStatus::kOk;
  HttpCode ret = HttpCode::kNoRequest;
  char* text = 0;

  while ((check_state_ == CheckState::kContent &&
          line_status == LineStatus::kOk) ||
         ((line_status = ParseLine()) == LineStatus::kOk)) {
    text = GetLine();
    start_line_ = checked_idx_;
    LOG_INFO("%s", text);
    switch (check_state_) {
      case CheckState::kRequestLine: {
        ret = ParseRequestLine(text);
        if (ret == HttpCode::kBadRequest) return HttpCode::kBadRequest;
        break;
      }
      case CheckState::kHeader: {
        ret = ParseHeaders(text);
        if (ret == HttpCode::kBadRequest)
          return HttpCode::kBadRequest;
        else if (ret == HttpCode::kGetRequest) {
          return DoRequest();
        }
        break;
      }
      case CheckState::kContent: {
        ret = ParseContent(text);
        if (ret == HttpCode::kGetRequest) return DoRequest();
        line_status = LineStatus::kOpen;
        break;
      }
      default:
        return HttpCode::kInternalError;
    }
  }
  return HttpCode::kNoRequest;
}

HttpConnection::HttpCode HttpConnection::DoRequest() {
  strcpy(&real_file_[0], doc_root_);
  int len = strlen(doc_root_);
  const char* p = strrchr(url_, '/');

  // 处理cgi
  if (cgi_ == 1 && (*(p + 1) == '2' || *(p + 1) == '3')) {
    // 根据标志判断是登录检测还是注册检测
    char flag = url_[1];

    char* m_url_real = (char*)malloc(sizeof(char) * 200);
    strcpy(m_url_real, "/");
    strcat(m_url_real, url_ + 2);
    strncpy(&real_file_[len], m_url_real, kFileNameLen - len - 1);
    free(m_url_real);

    // 将用户名和密码提取出来
    // user=123&passwd=123
    char name[100], password[100];
    int i;
    for (i = 5; string_[i] != '&'; ++i) name[i - 5] = string_[i];
    name[i - 5] = '\0';

    int j = 0;
    for (i = i + 10; string_[i] != '\0'; ++i, ++j) password[j] = string_[i];
    password[j] = '\0';

    if (*(p + 1) == '3') {
      // 如果是注册，先检测数据库中是否有重名的
      // 没有重名的，进行增加数据
      char* sql_insert = (char*)malloc(sizeof(char) * 200);
      strcpy(sql_insert, "INSERT INTO user(username, passwd) VALUES(");
      strcat(sql_insert, "'");
      strcat(sql_insert, name);
      strcat(sql_insert, "', '");
      strcat(sql_insert, password);
      strcat(sql_insert, "')");

      if (users.find(name) == users.end()) {
        std::lock_guard<std::mutex> lock(users_mutex);
        int res = mysql_query(mysql, sql_insert);
        users.insert(std::pair<std::string, std::string>(name, password));

        if (!res)
          strcpy(url_, "/log.html");
        else
          strcpy(url_, "/registerError.html");
      } else
        strcpy(url_, "/registerError.html");
    }
    // 如果是登录，直接判断
    // 若浏览器端输入的用户名和密码在表中可以查找到，返回1，否则返回0
    else if (*(p + 1) == '2') {
      if (users.find(name) != users.end() && users[name] == password)
        strcpy(url_, "/welcome.html");
      else
        strcpy(url_, "/logError.html");
    }
  }

  if (*(p + 1) == '0') {
    char* m_url_real = (char*)malloc(sizeof(char) * 200);
    strcpy(m_url_real, "/register.html");
    strncpy(&real_file_[len], m_url_real, strlen(m_url_real));

    free(m_url_real);
  } else if (*(p + 1) == '1') {
    char* m_url_real = (char*)malloc(sizeof(char) * 200);
    strcpy(m_url_real, "/log.html");
    strncpy(&real_file_[len], m_url_real, strlen(m_url_real));

    free(m_url_real);
  } else if (*(p + 1) == '5') {
    char* m_url_real = (char*)malloc(sizeof(char) * 200);
    strcpy(m_url_real, "/picture.html");
    strncpy(&real_file_[len], m_url_real, strlen(m_url_real));

    free(m_url_real);
  } else if (*(p + 1) == '6') {
    char* m_url_real = (char*)malloc(sizeof(char) * 200);
    strcpy(m_url_real, "/video.html");
    strncpy(&real_file_[len], m_url_real, strlen(m_url_real));

    free(m_url_real);
  } else if (*(p + 1) == '7') {
    char* m_url_real = (char*)malloc(sizeof(char) * 200);
    strcpy(m_url_real, "/fans.html");
    strncpy(&real_file_[len], m_url_real, strlen(m_url_real));

    free(m_url_real);
  } else
    strncpy(&real_file_[len], url_, kFileNameLen - len - 1);

  if (stat(&real_file_[0], &file_stat_) < 0) return HttpCode::kNoResource;

  if (!(file_stat_.st_mode & S_IROTH)) return HttpCode::kForbiddenRequest;

  if (S_ISDIR(file_stat_.st_mode)) return HttpCode::kBadRequest;

  int fd = open(&real_file_[0], O_RDONLY);
  file_address_ =
      (char*)mmap(0, file_stat_.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  close(fd);
  return HttpCode::kFileRequest;
}

void HttpConnection::Unmap() {
  if (file_address_) {
    munmap(file_address_, file_stat_.st_size);
    file_address_ = 0;
  }
}

bool HttpConnection::write() {
  int temp = 0;

  if (bytes_to_send_ == 0) {
    ModifyFd(m_epollfd, sockfd_, EPOLLIN, trigger_mode_);
    init();
    return true;
  }

  while (1) {
    temp = writev(sockfd_, iov_, iov_count_);

    if (temp < 0) {
      if (errno == EAGAIN) {
        ModifyFd(m_epollfd, sockfd_, EPOLLOUT, trigger_mode_);
        return true;
      }
      Unmap();
      return false;
    }

    bytes_have_send_ += temp;
    bytes_to_send_ -= temp;
    if (bytes_have_send_ >= iov_[0].iov_len) {
      iov_[0].iov_len = 0;
      iov_[1].iov_base = file_address_ + (bytes_have_send_ - write_idx_);
      iov_[1].iov_len = bytes_to_send_;
    } else {
      iov_[0].iov_base = &write_buf_[bytes_have_send_];
      iov_[0].iov_len = iov_[0].iov_len - bytes_have_send_;
    }

    if (bytes_to_send_ <= 0) {
      Unmap();
      ModifyFd(m_epollfd, sockfd_, EPOLLIN, trigger_mode_);

      if (linger_) {
        init();
        return true;
      } else {
        return false;
      }
    }
  }
}

bool HttpConnection::AddResponse(const char* format, ...) {
  if (write_idx_ >= kWriteBufferSize) return false;
  va_list arg_list;
  va_start(arg_list, format);
  int len = vsnprintf(&write_buf_[write_idx_], kWriteBufferSize - 1 - write_idx_,
                      format, arg_list);
  if (len >= (kWriteBufferSize - 1 - write_idx_)) {
    va_end(arg_list);
    return false;
  }
  write_idx_ += len;
  va_end(arg_list);

  LOG_INFO("request:%s", &write_buf_[0]);

  return true;
}

bool HttpConnection::AddStatusLine(int status, const char* title) {
  return AddResponse("%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool HttpConnection::AddHeaders(int content_len) {
  return AddContentLength(content_len) && AddLinger() && AddBlankLine();
}

bool HttpConnection::AddContentLength(int content_len) {
  return AddResponse("Content-Length:%d\r\n", content_len);
}

bool HttpConnection::AddContentType() {
  return AddResponse("Content-Type:%s\r\n", "text/html");
}

bool HttpConnection::AddLinger() {
  return AddResponse("Connection:%s\r\n",
                     (linger_ == true) ? "keep-alive" : "close");
}

bool HttpConnection::AddBlankLine() { return AddResponse("%s", "\r\n"); }

bool HttpConnection::AddContent(const char* content) {
  return AddResponse("%s", content);
}

bool HttpConnection::ProcessWrite(HttpCode ret) {
  switch (ret) {
    case HttpCode::kInternalError: {
      AddStatusLine(500, kError500Title);
      AddHeaders(strlen(kError500Form));
      if (!AddContent(kError500Form)) return false;
      break;
    }
    case HttpCode::kBadRequest: {
      AddStatusLine(404, kError404Title);
      AddHeaders(strlen(kError404Form));
      if (!AddContent(kError404Form)) return false;
      break;
    }
    case HttpCode::kForbiddenRequest: {
      AddStatusLine(403, kError403Title);
      AddHeaders(strlen(kError403Form));
      if (!AddContent(kError403Form)) return false;
      break;
    }
    case HttpCode::kFileRequest: {
      AddStatusLine(200, kOk200Title);
      if (file_stat_.st_size != 0) {
        AddHeaders(file_stat_.st_size);
        iov_[0].iov_base = &write_buf_[0];
        iov_[0].iov_len = write_idx_;
        iov_[1].iov_base = file_address_;
        iov_[1].iov_len = file_stat_.st_size;
        iov_count_ = 2;
        bytes_to_send_ = write_idx_ + file_stat_.st_size;
        return true;
      } else {
        const char* ok_string = "<html><body></body></html>";
        AddHeaders(strlen(ok_string));
        if (!AddContent(ok_string)) return false;
      }
    }
    default:
      return false;
  }
  iov_[0].iov_base = &write_buf_[0];
  iov_[0].iov_len = write_idx_;
  iov_count_ = 1;
  bytes_to_send_ = write_idx_;
  return true;
}

void HttpConnection::process() {
  HttpCode read_ret = ProcessRead();
  if (read_ret == HttpCode::kNoRequest) {
    ModifyFd(m_epollfd, sockfd_, EPOLLIN, trigger_mode_);
    return;
  }
  bool write_ret = ProcessWrite(read_ret);
  if (!write_ret) {
    close_conn();
  }
  ModifyFd(m_epollfd, sockfd_, EPOLLOUT, trigger_mode_);
}

// Helper function for timer callback
void DecrementHttpUserCount() { HttpConnection::m_user_count--; }

}  // namespace tinywebserver
