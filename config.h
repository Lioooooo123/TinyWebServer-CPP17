// Copyright 2025 TinyWebServer
// Configuration management system with command-line and file support
// Follows Google C++ Style Guide

#ifndef TINYWEBSERVER_CONFIG_H_
#define TINYWEBSERVER_CONFIG_H_

#include <optional>
#include <string>
#include <unordered_map>

namespace tinywebserver {

// Configuration class for server settings.
// Supports loading from command-line arguments and configuration files.
class Config {
 public:
  Config();
  ~Config() = default;

  // Disable copy operations
  Config(const Config&) = delete;
  Config& operator=(const Config&) = delete;

  // Allow move operations
  Config(Config&&) = default;
  Config& operator=(Config&&) = default;

  // Parses command-line arguments.
  // @param argc Argument count
  // @param argv Argument vector
  void ParseArgs(int argc, char* argv[]);

  // Loads configuration from file.
  // @param config_file Path to configuration file
  // @return true if successful, false otherwise
  bool LoadFromFile(const std::string& config_file);

  // Validates the configuration values.
  // @return true if valid, false otherwise
  bool Validate() const;

  // Prints current configuration to stdout
  void Print() const;

  // Getters
  int port() const { return port_; }
  int log_write_mode() const { return log_write_mode_; }
  int trigger_mode() const { return trigger_mode_; }
  int listen_trigger_mode() const { return listen_trigger_mode_; }
  int conn_trigger_mode() const { return conn_trigger_mode_; }
  int opt_linger() const { return opt_linger_; }
  int sql_connection_num() const { return sql_connection_num_; }
  int thread_num() const { return thread_num_; }
  int close_log() const { return close_log_; }
  int actor_model() const { return actor_model_; }

  // Setters (for testing and programmatic configuration)
  void set_port(int port) { port_ = port; }
  void set_log_write_mode(int mode) { log_write_mode_ = mode; }
  void set_trigger_mode(int mode) { trigger_mode_ = mode; }
  void set_sql_connection_num(int num) { sql_connection_num_ = num; }
  void set_thread_num(int num) { thread_num_ = num; }
  void set_close_log(int flag) { close_log_ = flag; }
  void set_actor_model(int model) { actor_model_ = model; }

 private:
  // Parses a single configuration key-value pair
  void ParseConfigLine(const std::string& key, const std::string& value);

  // Helper to parse integer with validation
  std::optional<int> ParseInt(const std::string& str) const;

  int port_;                    // Server port number
  int log_write_mode_;          // Log write mode (0=sync, 1=async)
  int trigger_mode_;            // Trigger combination mode
  int listen_trigger_mode_;     // Listen fd trigger mode
  int conn_trigger_mode_;       // Connection fd trigger mode
  int opt_linger_;              // SO_LINGER option
  int sql_connection_num_;      // Database connection pool size
  int thread_num_;              // Thread pool size
  int close_log_;               // Log disable flag (0=enable, 1=disable)
  int actor_model_;             // Concurrency model (0=proactor, 1=reactor)
};

}  // namespace tinywebserver

#endif  // TINYWEBSERVER_CONFIG_H_
