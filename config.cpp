// Copyright 2025 TinyWebServer
// 配置管理系统的实现

#include "config.h"

#include <getopt.h>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace tinywebserver {

namespace {

// 去除字符串两端的空白字符
std::string Trim(const std::string& str) {
  auto first = std::find_if_not(str.begin(), str.end(),
                                 [](unsigned char ch) { return std::isspace(ch); });
  if (first == str.end()) {
    return "";
  }
  auto last = std::find_if_not(str.rbegin(), str.rend(),
                                [](unsigned char ch) { return std::isspace(ch); })
                  .base();
  return std::string(first, last);
}

// 解析整数并进行错误处理
int ParseOrDefault(const char* arg, int fallback, char flag) {
  if (arg == nullptr) {
    return fallback;
  }

  try {
    return std::stoi(arg);
  } catch (const std::exception& e) {
    std::cerr << "[Config] Invalid argument value: -" << flag << "=" << arg
              << ", using default " << fallback << std::endl;
    return fallback;
  }
}

}  // namespace

Config::Config()
    : port_(9006),
      log_write_mode_(0),
      trigger_mode_(0),
      listen_trigger_mode_(0),
      conn_trigger_mode_(0),
      opt_linger_(0),
      sql_connection_num_(8),
      thread_num_(8),
      close_log_(0),
      actor_model_(0) {}

void Config::ParseArgs(int argc, char* argv[]) {
  int opt = 0;
  constexpr const char* kOptString = "p:l:m:o:s:t:c:a:f:h";

  while ((opt = getopt(argc, argv, kOptString)) != -1) {
    switch (opt) {
      case 'p':
        port_ = ParseOrDefault(optarg, port_, 'p');
        break;
      case 'l':
        log_write_mode_ = ParseOrDefault(optarg, log_write_mode_, 'l');
        break;
      case 'm':
        trigger_mode_ = ParseOrDefault(optarg, trigger_mode_, 'm');
        break;
      case 'o':
        opt_linger_ = ParseOrDefault(optarg, opt_linger_, 'o');
        break;
      case 's':
        sql_connection_num_ = ParseOrDefault(optarg, sql_connection_num_, 's');
        break;
      case 't':
        thread_num_ = ParseOrDefault(optarg, thread_num_, 't');
        break;
      case 'c':
        close_log_ = ParseOrDefault(optarg, close_log_, 'c');
        break;
      case 'a':
        actor_model_ = ParseOrDefault(optarg, actor_model_, 'a');
        break;
      case 'f':
        if (optarg) {
          LoadFromFile(optarg);
        }
        break;
      case 'h':
        std::cout << "Usage: " << argv[0] << " [options]\n"
                  << "Options:\n"
                  << "  -p <port>         Server port (default: 9006)\n"
                  << "  -l <mode>         Log write mode 0=sync 1=async (default: 0)\n"
                  << "  -m <mode>         Trigger mode (default: 0)\n"
                  << "  -o <flag>         SO_LINGER option (default: 0)\n"
                  << "  -s <num>          SQL connection pool size (default: 8)\n"
                  << "  -t <num>          Thread pool size (default: 8)\n"
                  << "  -c <flag>         Close log 0=enable 1=disable (default: 0)\n"
                  << "  -a <model>        Actor model 0=proactor 1=reactor (default: 0)\n"
                  << "  -f <file>         Load config from file\n"
                  << "  -h                Show this help message\n";
        break;
      default:
        break;
    }
  }

  // 解析后进行验证
  if (!Validate()) {
    std::cerr << "[Config] Configuration validation failed" << std::endl;
  }
}

bool Config::LoadFromFile(const std::string& config_file) {
  std::ifstream infile(config_file);
  if (!infile.is_open()) {
    std::cerr << "[Config] Unable to open config file: " << config_file << std::endl;
    return false;
  }

  std::string line;
  int line_number = 0;
  while (std::getline(infile, line)) {
    ++line_number;
    line = Trim(line);
    
    // 跳过空行和注释
    if (line.empty() || line[0] == '#') {
      continue;
    }

    size_t eq_pos = line.find('=');
    if (eq_pos == std::string::npos) {
      std::cerr << "[Config] Invalid format at line " << line_number 
                << ": " << line << std::endl;
      continue;
    }

    std::string key = Trim(line.substr(0, eq_pos));
    std::string value = Trim(line.substr(eq_pos + 1));

    ParseConfigLine(key, value);
  }

  infile.close();
  std::cout << "[Config] Loaded configuration from " << config_file << std::endl;
  return Validate();
}

void Config::ParseConfigLine(const std::string& key, const std::string& value) {
  auto int_value = ParseInt(value);
  if (!int_value) {
    std::cerr << "[Config] Invalid integer value for " << key 
              << ": " << value << std::endl;
    return;
  }

  if (key == "PORT" || key == "port") {
    port_ = *int_value;
  } else if (key == "LOGWrite" || key == "log_write_mode") {
    log_write_mode_ = *int_value;
  } else if (key == "TRIGMode" || key == "trigger_mode") {
    trigger_mode_ = *int_value;
  } else if (key == "LISTENTrigmode" || key == "listen_trigger_mode") {
    listen_trigger_mode_ = *int_value;
  } else if (key == "CONNTrigmode" || key == "conn_trigger_mode") {
    conn_trigger_mode_ = *int_value;
  } else if (key == "OPT_LINGER" || key == "opt_linger") {
    opt_linger_ = *int_value;
  } else if (key == "sql_num" || key == "sql_connection_num") {
    sql_connection_num_ = *int_value;
  } else if (key == "thread_num") {
    thread_num_ = *int_value;
  } else if (key == "close_log") {
    close_log_ = *int_value;
  } else if (key == "actor_model") {
    actor_model_ = *int_value;
  } else {
    std::cerr << "[Config] Unknown configuration key: " << key << std::endl;
  }
}

std::optional<int> Config::ParseInt(const std::string& str) const {
  try {
    return std::stoi(str);
  } catch (const std::exception&) {
    return std::nullopt;
  }
}

bool Config::Validate() const {
  bool valid = true;

  if (port_ < 1024 || port_ > 65535) {
    std::cerr << "[Config] Invalid port number: " << port_ 
              << " (must be between 1024 and 65535)" << std::endl;
    valid = false;
  }

  if (log_write_mode_ < 0 || log_write_mode_ > 1) {
    std::cerr << "[Config] Invalid log_write_mode: " << log_write_mode_
              << " (must be 0 or 1)" << std::endl;
    valid = false;
  }

  if (sql_connection_num_ <= 0 || sql_connection_num_ > 100) {
    std::cerr << "[Config] Invalid sql_connection_num: " << sql_connection_num_
              << " (must be between 1 and 100)" << std::endl;
    valid = false;
  }

  if (thread_num_ <= 0 || thread_num_ > 100) {
    std::cerr << "[Config] Invalid thread_num: " << thread_num_
              << " (must be between 1 and 100)" << std::endl;
    valid = false;
  }

  if (actor_model_ < 0 || actor_model_ > 1) {
    std::cerr << "[Config] Invalid actor_model: " << actor_model_
              << " (must be 0 or 1)" << std::endl;
    valid = false;
  }

  return valid;
}

void Config::Print() const {
  std::cout << "=== Server Configuration ===" << std::endl;
  std::cout << "Port:                " << port_ << std::endl;
  std::cout << "Log Write Mode:      " << log_write_mode_ 
            << (log_write_mode_ == 0 ? " (sync)" : " (async)") << std::endl;
  std::cout << "Trigger Mode:        " << trigger_mode_ << std::endl;
  std::cout << "Listen Trigger Mode: " << listen_trigger_mode_ << std::endl;
  std::cout << "Conn Trigger Mode:   " << conn_trigger_mode_ << std::endl;
  std::cout << "Opt Linger:          " << opt_linger_ << std::endl;
  std::cout << "SQL Connections:     " << sql_connection_num_ << std::endl;
  std::cout << "Thread Pool Size:    " << thread_num_ << std::endl;
  std::cout << "Log Disabled:        " << close_log_ 
            << (close_log_ == 0 ? " (enabled)" : " (disabled)") << std::endl;
  std::cout << "Actor Model:         " << actor_model_ 
            << (actor_model_ == 0 ? " (proactor)" : " (reactor)") << std::endl;
  std::cout << "===========================" << std::endl;
}

}  // namespace tinywebserver
