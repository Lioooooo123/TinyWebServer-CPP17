// Copyright 2025 TinyWebServer
// 使用循环数组实现的线程安全阻塞队列
// 遵循 Google C++ 编码规范

#ifndef TINYWEBSERVER_LOG_BLOCK_QUEUE_H_
#define TINYWEBSERVER_LOG_BLOCK_QUEUE_H_

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <stdexcept>

namespace tinywebserver {

// 使用循环数组的线程安全阻塞队列模板类。
// 所有操作都是线程安全的，由互斥锁保护。
template <typename T>
class BlockQueue {
 public:
  // 构造指定最大大小的 BlockQueue。
  // @param max_size 队列能容纳的最大元素数量
  // @throws std::invalid_argument 如果 max_size <= 0
  explicit BlockQueue(int max_size = 1000)
      : max_size_(max_size),
        size_(0),
        front_(-1),
        back_(-1),
        array_(new T[max_size]) {
    if (max_size <= 0) {
      throw std::invalid_argument("BlockQueue max_size must be positive");
    }
  }

  // 禁用拷贝和移动操作
  BlockQueue(const BlockQueue&) = delete;
  BlockQueue& operator=(const BlockQueue&) = delete;
  BlockQueue(BlockQueue&&) = delete;
  BlockQueue& operator=(BlockQueue&&) = delete;

  ~BlockQueue() = default;

  // 清空队列中的所有元素。
  void Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    size_ = 0;
    front_ = -1;
    back_ = -1;
  }

  // 检查队列是否已满。
  // @return 如果队列已满返回 true，否则返回 false
  bool Full() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return size_ >= max_size_;
  }

  // 检查队列是否为空。
  // @return 如果队列为空返回 true，否则返回 false
  bool Empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return size_ == 0;
  }

  // 获取队头元素但不移除它。
  // @param value 输出参数，用于存储队头元素
  // @return 如果成功返回 true，如果队列为空返回 false
  bool Front(T& value) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (size_ == 0) {
      return false;
    }
    value = array_[front_];
    return true;
  }

  // 获取队尾元素但不移除它。
  // @param value 输出参数，用于存储队尾元素
  // @return 如果成功返回 true，如果队列为空返回 false
  bool Back(T& value) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (size_ == 0) {
      return false;
    }
    value = array_[back_];
    return true;
  }

  // 返回队列中当前的元素数量。
  int Size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return size_;
  }

  // 返回队列的最大容量。
  int MaxSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return max_size_;
  }

  // 向队列中推入一个元素。
  // @param item 要推入的元素
  // @return 如果成功返回 true，如果队列已满返回 false
  bool Push(const T& item) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (size_ >= max_size_) {
      cond_.notify_all();
      return false;
    }

    back_ = (back_ + 1) % max_size_;
    array_[back_] = item;
    size_++;

    cond_.notify_all();
    return true;
  }

  // 从队列中弹出一个元素（阻塞调用）。
  // 等待直到有元素可用。
  // @param item 输出参数，用于存储弹出的元素
  // @return 如果成功返回 true，出错返回 false
  bool Pop(T& item) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (size_ <= 0) {
      cond_.wait(lock);
    }

    front_ = (front_ + 1) % max_size_;
    item = array_[front_];
    size_--;
    return true;
  }

  // 从队列中弹出一个元素（带超时）。
  // @param item 输出参数，用于存储弹出的元素
  // @param timeout_ms 超时时间（毫秒）
  // @return 如果成功返回 true，如果超时或队列为空返回 false
  bool Pop(T& item, int timeout_ms) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    if (size_ <= 0) {
      auto timeout = std::chrono::milliseconds(timeout_ms);
      if (cond_.wait_for(lock, timeout) == std::cv_status::timeout) {
        return false;
      }
    }

    if (size_ <= 0) {
      return false;
    }

    front_ = (front_ + 1) % max_size_;
    item = array_[front_];
    size_--;
    return true;
  }

 private:
  mutable std::mutex mutex_;
  std::condition_variable cond_;
  std::unique_ptr<T[]> array_;
  int size_;
  int max_size_;
  int front_;
  int back_;
};

}  // namespace tinywebserver

#endif  // TINYWEBSERVER_LOG_BLOCK_QUEUE_H_
