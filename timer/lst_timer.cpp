// Copyright 2025 TinyWebServer
// 定时器管理系统的实现

#include "lst_timer.h"

#include <errno.h>

#include <cassert>
#include <cstring>

namespace tinywebserver {

// 来自 http 模块的前向声明
namespace {
// 来自 http_conn 模块的外部用户计数
extern int g_user_count;
}  // namespace

// SortedTimerList 实现

SortedTimerList::SortedTimerList() : head_(nullptr), tail_(nullptr) {}

SortedTimerList::~SortedTimerList() {
  Timer* current = head_;
  while (current) {
    head_ = current->next_;
    delete current;
    current = head_;
  }
}

void SortedTimerList::AddTimer(Timer* timer) {
  if (!timer) {
    return;
  }

  if (!head_) {
    head_ = tail_ = timer;
    return;
  }

  if (timer->expire_time_ < head_->expire_time_) {
    timer->next_ = head_;
    head_->prev_ = timer;
    head_ = timer;
    return;
  }

  AddTimer(timer, head_);
}

void SortedTimerList::AdjustTimer(Timer* timer) {
  if (!timer) {
    return;
  }

  Timer* next_timer = timer->next_;
  if (!next_timer || (timer->expire_time_ < next_timer->expire_time_)) {
    return;
  }

  if (timer == head_) {
    head_ = head_->next_;
    head_->prev_ = nullptr;
    timer->next_ = nullptr;
    AddTimer(timer, head_);
  } else {
    timer->prev_->next_ = timer->next_;
    timer->next_->prev_ = timer->prev_;
    AddTimer(timer, timer->next_);
  }
}

void SortedTimerList::DeleteTimer(Timer* timer) {
  if (!timer) {
    return;
  }

  if ((timer == head_) && (timer == tail_)) {
    delete timer;
    head_ = nullptr;
    tail_ = nullptr;
    return;
  }

  if (timer == head_) {
    head_ = head_->next_;
    head_->prev_ = nullptr;
    delete timer;
    return;
  }

  if (timer == tail_) {
    tail_ = tail_->prev_;
    tail_->next_ = nullptr;
    delete timer;
    return;
  }

  timer->prev_->next_ = timer->next_;
  timer->next_->prev_ = timer->prev_;
  delete timer;
}

void SortedTimerList::Tick() {
  if (!head_) {
    return;
  }

  auto now = std::chrono::steady_clock::now();
  Timer* current = head_;

  while (current) {
    if (now < current->expire_time_) {
      break;
    }

    if (current->callback_) {
      current->callback_(current->user_data_);
    }

    head_ = current->next_;
    if (head_) {
      head_->prev_ = nullptr;
    }
    delete current;
    current = head_;
  }
}

void SortedTimerList::AddTimer(Timer* timer, Timer* list_head) {
  Timer* prev = list_head;
  Timer* current = prev->next_;

  while (current) {
    if (timer->expire_time_ < current->expire_time_) {
      prev->next_ = timer;
      timer->next_ = current;
      current->prev_ = timer;
      timer->prev_ = prev;
      break;
    }
    prev = current;
    current = current->next_;
  }

  if (!current) {
    prev->next_ = timer;
    timer->prev_ = prev;
    timer->next_ = nullptr;
    tail_ = timer;
  }
}

// TimerUtils 实现

void TimerUtils::Init(int timeslot) {
  timeslot_ = timeslot;
}

int TimerUtils::SetNonBlocking(int fd) {
  int old_option = fcntl(fd, F_GETFL);
  int new_option = old_option | O_NONBLOCK;
  fcntl(fd, F_SETFL, new_option);
  return old_option;
}

void TimerUtils::AddFd(int epollfd, int fd, bool one_shot, int trigger_mode) {
  epoll_event event;
  event.data.fd = fd;

  if (trigger_mode == 1) {
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
  } else {
    event.events = EPOLLIN | EPOLLRDHUP;
  }

  if (one_shot) {
    event.events |= EPOLLONESHOT;
  }

  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
  SetNonBlocking(fd);
}

void TimerUtils::SignalHandler(int sig) {
  // 立即输出，不管其他
  const char* msg_str = "[SIGNAL HANDLER CALLED]\n";
  write(STDERR_FILENO, msg_str, strlen(msg_str));
  
  // 保存 errno 以保证可重入性
  int saved_errno = errno;
  int msg = sig;
  
  if (pipe_fd_ != nullptr && pipe_fd_[1] >= 0) {
    send(pipe_fd_[1], reinterpret_cast<char*>(&msg), 1, 0);
  }
  
  errno = saved_errno;
}

void TimerUtils::AddSignal(int sig, void (*handler)(int), bool restart) {
  struct sigaction sa;
  std::memset(&sa, 0, sizeof(sa));
  sa.sa_handler = handler;

  if (restart) {
    sa.sa_flags |= SA_RESTART;
  }

  sigfillset(&sa.sa_mask);
  int ret = sigaction(sig, &sa, nullptr);
  
  fprintf(stderr, "[AddSignal] Registering signal %d, handler=%p, result=%d\n", 
          sig, (void*)handler, ret);
  fflush(stderr);
  
  assert(ret != -1);
}

void TimerUtils::HandleTimer() {
  timer_list_.Tick();
  alarm(timeslot_);
}

void TimerUtils::ShowError(int connfd, const char* info) {
  send(connfd, info, std::strlen(info), 0);
  close(connfd);
}

// 静态成员初始化
int* TimerUtils::pipe_fd_ = nullptr;
int TimerUtils::epoll_fd_ = 0;

// 定时器回调函数
void TimerCallback(ClientData* user_data) {
  if (!user_data) {
    return;
  }

  epoll_ctl(TimerUtils::epoll_fd_, EPOLL_CTL_DEL, user_data->sockfd, nullptr);
  close(user_data->sockfd);
  
  // 减少用户计数（将在链接时与 http_conn 模块正确链接）
  // 这是一个前向声明，将在链接时解析
  extern void DecrementHttpUserCount();
  DecrementHttpUserCount();
}

}  // namespace tinywebserver

