// Copyright 2025 TinyWebServer
// 异步日志系统的实现

#include "log.h"

#include <sys/time.h>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace tinywebserver {

Logger::Logger()
    : split_lines_(0),
      log_buf_size_(0),
      count_(0),
      today_(0),
      is_async_(false),
      close_log_(0) {}

Logger::~Logger() {
  if (fp_) {
    std::fflush(fp_.get());
  }
  
  if (async_thread_ && async_thread_->joinable()) {
    // 通过推入空字符串来通知异步线程停止
    if (log_queue_) {
      log_queue_->Push("");
    }
    async_thread_->join();
  }
}

bool Logger::Init(const std::string& file_name, int close_log,
                  int log_buf_size, int split_lines, int max_queue_size) {
  // 如果指定了队列大小，设置异步模式
  if (max_queue_size >= 1) {
    is_async_ = true;
    log_queue_ = std::make_unique<BlockQueue<std::string>>(max_queue_size);
    
    // 创建异步写入线程
    async_thread_ = std::make_unique<std::thread>([this]() {
      this->AsyncWriteLog();
    });
  }

  close_log_ = close_log;
  log_buf_size_ = log_buf_size;
  buf_ = std::make_unique<char[]>(log_buf_size);
  std::memset(buf_.get(), '\0', log_buf_size);
  split_lines_ = split_lines;

  // 获取当前时间
  time_t t = time(nullptr);
  struct tm* sys_tm = localtime(&t);
  struct tm my_tm = *sys_tm;

  // 解析文件路径
  std::filesystem::path file_path(file_name);
  std::filesystem::path parent_path = file_path.parent_path();
  std::filesystem::path filename = file_path.filename();

  // 生成带日期的日志文件名
  std::ostringstream log_full_name;
  if (parent_path.empty()) {
    log_full_name << my_tm.tm_year + 1900 << "_"
                  << std::setfill('0') << std::setw(2) << my_tm.tm_mon + 1 << "_"
                  << std::setfill('0') << std::setw(2) << my_tm.tm_mday << "_"
                  << filename.string();
    log_name_ = filename;
  } else {
    log_full_name << parent_path.string() << "/"
                  << my_tm.tm_year + 1900 << "_"
                  << std::setfill('0') << std::setw(2) << my_tm.tm_mon + 1 << "_"
                  << std::setfill('0') << std::setw(2) << my_tm.tm_mday << "_"
                  << filename.string();
    dir_name_ = parent_path;
    log_name_ = filename;
    
    // 如果目录不存在则创建
    std::filesystem::create_directories(parent_path);
  }

  today_ = my_tm.tm_mday;

  // 打开日志文件
  fp_.reset(std::fopen(log_full_name.str().c_str(), "a"));
  if (!fp_) {
    return false;
  }

  return true;
}

void Logger::WriteLog(LogLevel level, const char* format, ...) {
  struct timeval now = {0, 0};
  gettimeofday(&now, nullptr);
  time_t t = now.tv_sec;
  struct tm* sys_tm = localtime(&t);
  struct tm my_tm = *sys_tm;

  // 获取级别字符串
  const char* level_str = GetLevelString(level);

  // 处理日志切分
  std::unique_lock<std::mutex> lock(mutex_);
  count_++;

  if (today_ != my_tm.tm_mday || count_ % split_lines_ == 0) {
    std::fflush(fp_.get());
    fp_.reset();

    std::ostringstream new_log;
    std::ostringstream tail;
    tail << my_tm.tm_year + 1900 << "_"
         << std::setfill('0') << std::setw(2) << my_tm.tm_mon + 1 << "_"
         << std::setfill('0') << std::setw(2) << my_tm.tm_mday << "_";

    if (today_ != my_tm.tm_mday) {
      if (dir_name_.empty()) {
        new_log << tail.str() << log_name_.string();
      } else {
        new_log << dir_name_.string() << "/" << tail.str() << log_name_.string();
      }
      today_ = my_tm.tm_mday;
      count_ = 0;
    } else {
      if (dir_name_.empty()) {
        new_log << tail.str() << log_name_.string() << "." 
                << count_ / split_lines_;
      } else {
        new_log << dir_name_.string() << "/" << tail.str() 
                << log_name_.string() << "." << count_ / split_lines_;
      }
    }
    
    fp_.reset(std::fopen(new_log.str().c_str(), "a"));
  }

  lock.unlock();

  // 格式化日志消息
  va_list valst;
  va_start(valst, format);

  std::string log_str;
  lock.lock();

  // 写入时间戳和级别
  int n = std::snprintf(buf_.get(), 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                        my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                        my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, 
                        now.tv_usec, level_str);

  // 写入实际的日志内容
  int m = std::vsnprintf(buf_.get() + n, log_buf_size_ - n - 1, format, valst);
  buf_[n + m] = '\n';
  buf_[n + m + 1] = '\0';
  log_str = buf_.get();

  lock.unlock();

  // 写入异步队列或直接写入文件
  if (is_async_ && log_queue_ && !log_queue_->Full()) {
    log_queue_->Push(log_str);
  } else {
    lock.lock();
    std::fputs(log_str.c_str(), fp_.get());
    lock.unlock();
  }

  va_end(valst);
}

void Logger::Flush() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (fp_) {
    std::fflush(fp_.get());
  }
}

void Logger::AsyncWriteLog() {
  std::string single_log;
  // 从队列中弹出日志并写入文件
  while (log_queue_->Pop(single_log)) {
    // 空字符串表示关闭信号
    if (single_log.empty()) {
      break;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    if (fp_) {
      std::fputs(single_log.c_str(), fp_.get());
    }
  }
}

const char* Logger::GetLevelString(LogLevel level) const {
  switch (level) {
    case LogLevel::kDebug:
      return "[DEBUG]:";
    case LogLevel::kInfo:
      return "[INFO]:";
    case LogLevel::kWarn:
      return "[WARN]:";
    case LogLevel::kError:
      return "[ERROR]:";
    default:
      return "[INFO]:";
  }
}

}  // namespace tinywebserver
