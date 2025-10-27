// Copyright 2025 TinyWebServer
// 支持同步和异步模式的异步日志系统
// 遵循 Google C++ 编码规范

#ifndef TINYWEBSERVER_LOG_LOG_H_
#define TINYWEBSERVER_LOG_LOG_H_

#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "block_queue.h"

namespace tinywebserver {

// 日志级别枚举
enum class LogLevel {
  kDebug = 0,
  kInfo = 1,
  kWarn = 2,
  kError = 3
};

// 单例日志类，支持同步和异步日志记录。
// 线程安全，支持基于日期和行数的自动日志文件切分。
class Logger {
 public:
  // 获取 Logger 的单例实例
  static Logger* GetInstance() {
    static Logger instance;
    return &instance;
  }

  // 禁用拷贝和移动操作
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(Logger&&) = delete;

  // 使用配置参数初始化日志系统。
  // @param file_name 日志文件路径
  // @param close_log 禁用日志标志 (0 = 启用, 1 = 禁用)
  // @param log_buf_size 日志缓冲区大小（字节）
  // @param split_lines 文件切分前每个日志文件的最大行数
  // @param max_queue_size 异步队列大小 (0 = 同步模式, >0 = 异步模式)
  // @return 如果初始化成功返回 true，否则返回 false
  bool Init(const std::string& file_name, int close_log,
            int log_buf_size = 8192, int split_lines = 5000000,
            int max_queue_size = 0);

  // 写入指定级别和格式的日志消息。
  // @param level 日志级别 (debug, info, warn, error)
  // @param format Printf 风格的格式字符串
  // @param ... 格式字符串的可变参数
  void WriteLog(LogLevel level, const char* format, ...);

  // 将日志缓冲区刷新到磁盘。
  void Flush();

  // 检查日志是否被禁用。
  bool IsLogClosed() const { return close_log_ != 0; }

 private:
  Logger();
  ~Logger();

  // 异步日志写入线程函数。
  void AsyncWriteLog();

  // 获取日志级别的字符串表示。
  const char* GetLevelString(LogLevel level) const;

 private:
  std::filesystem::path dir_name_;     // 目录路径
  std::filesystem::path log_name_;     // 日志文件名
  int split_lines_;                     // 每个日志文件的最大行数
  int log_buf_size_;                    // 日志缓冲区大小
  long long count_;                     // 当前行数
  int today_;                           // 用于日志切分的当前日期
  std::unique_ptr<std::FILE, decltype(&std::fclose)> fp_{nullptr, &std::fclose};
  std::unique_ptr<char[]> buf_;
  std::unique_ptr<BlockQueue<std::string>> log_queue_;
  bool is_async_;                       // 异步模式标志
  std::mutex mutex_;
  int close_log_;                       // 日志禁用标志
  std::unique_ptr<std::thread> async_thread_;
};

}  // namespace tinywebserver

// 便捷的日志记录宏
#define LOG_DEBUG(format, ...)                                           \
  do {                                                                   \
    auto* logger = tinywebserver::Logger::GetInstance();                \
    if (!logger->IsLogClosed()) {                                       \
      logger->WriteLog(tinywebserver::LogLevel::kDebug, format,        \
                       ##__VA_ARGS__);                                   \
      logger->Flush();                                                   \
    }                                                                    \
  } while (0)

#define LOG_INFO(format, ...)                                            \
  do {                                                                   \
    auto* logger = tinywebserver::Logger::GetInstance();                \
    if (!logger->IsLogClosed()) {                                       \
      logger->WriteLog(tinywebserver::LogLevel::kInfo, format,         \
                       ##__VA_ARGS__);                                   \
      logger->Flush();                                                   \
    }                                                                    \
  } while (0)

#define LOG_WARN(format, ...)                                            \
  do {                                                                   \
    auto* logger = tinywebserver::Logger::GetInstance();                \
    if (!logger->IsLogClosed()) {                                       \
      logger->WriteLog(tinywebserver::LogLevel::kWarn, format,         \
                       ##__VA_ARGS__);                                   \
      logger->Flush();                                                   \
    }                                                                    \
  } while (0)

#define LOG_ERROR(format, ...)                                           \
  do {                                                                   \
    auto* logger = tinywebserver::Logger::GetInstance();                \
    if (!logger->IsLogClosed()) {                                       \
      logger->WriteLog(tinywebserver::LogLevel::kError, format,        \
                       ##__VA_ARGS__);                                   \
      logger->Flush();                                                   \
    }                                                                    \
  } while (0)

#endif  // TINYWEBSERVER_LOG_LOG_H_
