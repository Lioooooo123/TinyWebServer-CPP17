// Copyright 2025 TinyWebServer
// Main entry point for TinyWebServer application
// Follows Google C++ Style Guide

#include "config.h"
#include "webserver.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
  // Database configuration - should be modified according to your setup
  const std::string kDbUser = "root";
  const std::string kDbPassword = "root";
  const std::string kDbName = "Liodb";

  try {
    // Parse command-line arguments and configuration file
    tinywebserver::Config config;
    config.ParseArgs(argc, argv);

    // Validate configuration
    if (!config.Validate()) {
      std::cerr << "Invalid configuration. Exiting." << std::endl;
      return 1;
    }

    // Print configuration for debugging
    config.Print();

    // Initialize web server with configuration
    tinywebserver::WebServer server;
    server.Init(
        config.port(), kDbUser, kDbPassword, kDbName,
        config.log_write_mode(), config.opt_linger(), 
        config.trigger_mode(), config.sql_connection_num(),
        config.thread_num(), config.close_log(), 
        config.actor_model());

    // Initialize subsystems
    std::cout << "[DEBUG] Initializing log system..." << std::endl;
    server.InitLog();
    std::cout << "[DEBUG] Log system initialized" << std::endl;
    
    std::cout << "[DEBUG] Initializing SQL pool..." << std::endl;
    server.InitSqlPool();
    std::cout << "[DEBUG] SQL pool initialized" << std::endl;
    
    std::cout << "[DEBUG] Initializing thread pool..." << std::endl;
    server.InitThreadPool();
    std::cout << "[DEBUG] Thread pool initialized" << std::endl;
    
    std::cout << "[DEBUG] Setting trigger mode..." << std::endl;
    server.SetTriggerMode();
    std::cout << "[DEBUG] Trigger mode set" << std::endl;
    
    std::cout << "[DEBUG] Starting listen..." << std::endl;
    server.StartListen();
    std::cout << "[DEBUG] Listen started" << std::endl;

    // Run the event loop
    std::cout << "[DEBUG] Entering event loop..." << std::endl;
    server.EventLoop();

  } catch (const std::exception& e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
