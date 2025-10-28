// Copyright 2025 TinyWebServer
// Timer management with sorted linked list for connection timeout handling
// Follows Google C++ Style Guide

#ifndef TINYWEBSERVER_TIMER_LST_TIMER_H_
#define TINYWEBSERVER_TIMER_LST_TIMER_H_

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <functional>
#include <memory>

#include "../log/log.h"

namespace tinywebserver {

// Forward declarations
class Timer;
class HttpConnection;

// Client connection data associated with a timer
struct ClientData {
  sockaddr_in address;
  int sockfd{-1};
  Timer* timer{nullptr};
};

// Timer node in the sorted timer list.
// Manages timeout for a single connection.
class Timer {
 public:
  using TimePoint = std::chrono::steady_clock::time_point;
  using Callback = std::function<void(ClientData*)>;

  Timer()
      : expire_time_(std::chrono::steady_clock::now()),
        callback_(nullptr),
        user_data_(nullptr),
        prev_(nullptr),
        next_(nullptr) {}

  ~Timer() = default;

  // Disable copy and move operations
  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;
  Timer(Timer&&) = delete;
  Timer& operator=(Timer&&) = delete;

  TimePoint expire_time_;  // Expiration time
  Callback callback_;      // Callback function when timer expires
  ClientData* user_data_;  // Associated client data
  Timer* prev_;            // Previous timer in list
  Timer* next_;            // Next timer in list
};

// Sorted timer list for managing connection timeouts.
// Timers are sorted by expiration time in ascending order.
class SortedTimerList {
 public:
  SortedTimerList();
  ~SortedTimerList();

  // Disable copy and move operations
  SortedTimerList(const SortedTimerList&) = delete;
  SortedTimerList& operator=(const SortedTimerList&) = delete;
  SortedTimerList(SortedTimerList&&) = delete;
  SortedTimerList& operator=(SortedTimerList&&) = delete;

  // Adds a new timer to the list.
  // @param timer Timer to add (must not be null)
  void AddTimer(Timer* timer);

  // Adjusts timer position after its expiration time changes.
  // @param timer Timer to adjust
  void AdjustTimer(Timer* timer);

  // Removes and deletes a timer from the list.
  // @param timer Timer to delete
  void DeleteTimer(Timer* timer);

  // Processes all expired timers.
  // Invokes callbacks for expired timers and removes them from the list.
  void Tick();

 private:
  // Adds timer to list starting from specified position.
  void AddTimer(Timer* timer, Timer* list_head);

  Timer* head_;  // Head of the timer list
  Timer* tail_;  // Tail of the timer list
};

// Utility class for managing epoll, signals, and timers
class TimerUtils {
 public:
  TimerUtils() : timeslot_(0) {}
  ~TimerUtils() = default;

  // Disable copy operations
  TimerUtils(const TimerUtils&) = delete;
  TimerUtils& operator=(const TimerUtils&) = delete;

  // Initializes the timer utilities with a timeslot duration.
  // @param timeslot Time interval in seconds for alarm signals
  void Init(int timeslot);

  // Sets a file descriptor to non-blocking mode.
  // @param fd File descriptor to modify
  // @return Previous file descriptor flags
  int SetNonBlocking(int fd);

  // Adds a file descriptor to the epoll instance.
  // @param epollfd Epoll file descriptor
  // @param fd File descriptor to add
  // @param one_shot Whether to use EPOLLONESHOT
  // @param trigger_mode 0=LT, 1=ET
  void AddFd(int epollfd, int fd, bool one_shot, int trigger_mode);

  // Signal handler function.
  // @param sig Signal number
  static void SignalHandler(int sig);

  // Registers a signal handler.
  // @param sig Signal number
  // @param handler Handler function
  // @param restart Whether to restart interrupted system calls
  void AddSignal(int sig, void (*handler)(int), bool restart = true);

  // Handles timer expiration and reschedules alarm.
  void HandleTimer();

  // Sends error message to client and closes connection.
  // @param connfd Client connection file descriptor
  // @param info Error message
  void ShowError(int connfd, const char* info);

  // Static members for signal handling
  static int* pipe_fd_;       // Pipe for signal notification
  static int epoll_fd_;       // Epoll file descriptor

  SortedTimerList timer_list_;  // Timer list
  int timeslot_;                // Timer interval in seconds
};

// Callback function for timer expiration.
// Closes the connection and decrements the user count.
// @param user_data Client data associated with the timer
void TimerCallback(ClientData* user_data);

}  // namespace tinywebserver

#endif  // TINYWEBSERVER_TIMER_LST_TIMER_H_

