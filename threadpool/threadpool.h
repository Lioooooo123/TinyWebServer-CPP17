// Copyright 2025 TinyWebServer
// 使用生产者-消费者模式处理请求的线程池
// 遵循 Google C++ 编码规范

#ifndef TINYWEBSERVER_THREADPOOL_THREADPOOL_H_
#define TINYWEBSERVER_THREADPOOL_THREADPOOL_H_

#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <exception>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

#include "../CGImysql/sql_connection_pool.h"

namespace tinywebserver {

// 用于处理并发请求的线程池模板类。
// 使用生产者-消费者模式与工作队列。
// @tparam T 要处理的任务/请求类型
template <typename T>
class ThreadPool {
 public:
  // 构造线程池。
  // @param actor_model 并发模型 (0=Proactor, 1=Reactor)
  // @param conn_pool 数据库连接池
  // @param thread_number 工作线程数量
  // @param max_requests 队列中最大待处理请求数
  // @throws std::invalid_argument 如果参数无效
  ThreadPool(int actor_model, ConnectionPool* conn_pool, int thread_number = 8,
             int max_requests = 10000);

  ~ThreadPool();

  // 禁用拷贝和移动操作
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  // 向工作队列添加请求 (Reactor 模式)。
  // @param request 指向请求的共享指针
  // @param state 请求状态 (0=读, 1=写)
  // @return 如果请求成功入队返回 true，否则返回 false
  bool Append(std::shared_ptr<T> request, int state);

  // 向工作队列添加请求 (Proactor 模式)。
  // @param request 指向请求的共享指针
  // @return 如果请求成功入队返回 true，否则返回 false
  bool AppendProactor(std::shared_ptr<T> request);

 private:
  // 工作线程函数。
  // 持续处理工作队列中的请求。
  void Run();

  int thread_number_;                       // 线程数量
  int max_requests_;                        // 最大待处理请求数
  std::vector<std::thread> threads_;        // 工作线程数组
  std::queue<std::shared_ptr<T>> work_queue_;  // 请求队列
  std::mutex queue_mutex_;                  // 队列保护互斥锁
  std::condition_variable queue_cond_;      // 信号通知条件变量
  ConnectionPool* conn_pool_;               // 数据库连接池
  int actor_model_;                         // 并发模型
  std::atomic<bool> stop_;                  // 停止标志
};

// 模板实现

template <typename T>
ThreadPool<T>::ThreadPool(int actor_model, ConnectionPool* conn_pool,
                          int thread_number, int max_requests)
    : actor_model_(actor_model),
      thread_number_(thread_number),
      max_requests_(max_requests),
      conn_pool_(conn_pool),
      stop_(false) {
  if (thread_number <= 0 || max_requests <= 0) {
    throw std::invalid_argument(
        "ThreadPool: thread_number and max_requests must be positive");
  }

  if (conn_pool == nullptr) {
    throw std::invalid_argument("ThreadPool: conn_pool cannot be null");
  }

  threads_.reserve(thread_number_);

  for (int i = 0; i < thread_number; ++i) {
    threads_.emplace_back([this]() { this->Run(); });
  }
}

template <typename T>
ThreadPool<T>::~ThreadPool() {
  stop_ = true;
  queue_cond_.notify_all();

  for (auto& thread : threads_) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

template <typename T>
bool ThreadPool<T>::Append(std::shared_ptr<T> request, int state) {
  if (!request) {
    return false;
  }

  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (work_queue_.size() >= static_cast<size_t>(max_requests_)) {
      return false;
    }

    request->m_state = state;
    work_queue_.push(request);
  }

  queue_cond_.notify_one();
  return true;
}

template <typename T>
bool ThreadPool<T>::AppendProactor(std::shared_ptr<T> request) {
  if (!request) {
    return false;
  }

  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (work_queue_.size() >= static_cast<size_t>(max_requests_)) {
      return false;
    }

    work_queue_.push(request);
  }

  queue_cond_.notify_one();
  return true;
}

template <typename T>
void ThreadPool<T>::Run() {
  while (!stop_) {
    std::shared_ptr<T> request;
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      queue_cond_.wait(lock,
                       [this]() { return stop_ || !work_queue_.empty(); });

      if (stop_ && work_queue_.empty()) {
        break;
      }

      if (!work_queue_.empty()) {
        request = work_queue_.front();
        work_queue_.pop();
      }
    }

    if (!request) {
      continue;
    }

    try {
      if (actor_model_ == 1) {
        // Reactor 模式：处理读/写事件
        if (request->m_state == 0) {
          // 读事件
          if (request->read_once()) {
            request->improv = 1;
            ConnectionRAII mysql_conn(&request->mysql, conn_pool_);
            request->process();
          } else {
            request->improv = 1;
            request->timer_flag = 1;
          }
        } else {
          // 写事件
          if (request->write()) {
            request->improv = 1;
          } else {
            request->improv = 1;
            request->timer_flag = 1;
          }
        }
      } else {
        // Proactor 模式：处理请求
        ConnectionRAII mysql_conn(&request->mysql, conn_pool_);
        request->process();
      }
    } catch (const std::exception& e) {
      std::fprintf(stderr, "Thread pool task exception: %s\n", e.what());
    }
  }
}

}  // namespace tinywebserver

#endif  // TINYWEBSERVER_THREADPOOL_THREADPOOL_H_

