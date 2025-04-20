#ifndef TIME_HPP
#define TIME_HPP

#include <sys/timerfd.h>
#include <unistd.h>
#include <sys/epoll.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <type_traits>
#include <utility>
#include <atomic>
#include <memory>

using VoidCallbackType = std::function<void ()>;

template<
  typename FunT,
  typename std::enable_if<std::is_same<FunT, VoidCallbackType>::value>::type* = nullptr,
  typename Clock = std::chrono::system_clock
>
class Timer {
public:

public:
  Timer(std::chrono::nanoseconds duration, FunT&& func)
  :period_(duration),
   call_back_(std::forward<FunT>(func)){
    start();
  }

  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;
  Timer(Timer&&) = delete;
  Timer& operator=(Timer&&) = delete;

  void start() {
    if(period_ < std::chrono::nanoseconds(0)) {
      throw std::runtime_error("period must be greater than 0");
    }

    timer_fd_ = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    if(timer_fd_ == -1) {
      throw std::runtime_error("timerfd_create failed");
    }
    struct itimerspec duration;
    duration.it_interval.tv_sec = period_.count() / 1000000000;
    duration.it_interval.tv_nsec = period_.count() % 1000000000;
    duration.it_value.tv_sec = period_.count() / 1000000000;
    duration.it_value.tv_nsec = period_.count() % 1000000000;
    int ret = timerfd_settime(timer_fd_, 0, &duration, nullptr);
    if(ret == -1) {
      throw std::runtime_error("timerfd_settime failed");
    }

    epoll_fd_ = epoll_create(1);
    if(epoll_fd_ == -1) {
      throw std::runtime_error("epoll_create1 failed");
    }
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = timer_fd_;
    ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, timer_fd_, &ev);
    if(ret == -1) {
      throw std::runtime_error("epoll_ctl failed");
    }

    work_thread_ = std::thread(&Timer::work, this);
  }

  void work() {
    while(true) {
      struct epoll_event ev;
      int ret = epoll_wait(epoll_fd_, &ev, 1, -1);
      if(ret == -1) {
        throw std::runtime_error("epoll_wait failed");
      }
      if(ev.data.fd == timer_fd_) {
        uint64_t num;
        ret = read(timer_fd_, &num, sizeof(num));
        if(ret == -1) {
          throw std::runtime_error("read failed");
        }
        execute_callback();
      }
    }
  }

  void execute_callback() {
    execute_call_back<>();
  }

  template<
    typename CallBack = FunT,
    typename std::enable_if<std::is_same<FunT, VoidCallbackType>::value>::type* = nullptr
  >
  void execute_call_back() {
    call_back_();
  }

  ~Timer() {
    if(timer_fd_ != -1) {
      close(timer_fd_);
    }
    if(epoll_fd_ != -1) {
      close(epoll_fd_);
    }
    if(work_thread_.joinable()) {
      work_thread_.join();
    }
  }

private:
  std::chrono::nanoseconds period_{-1};
  FunT call_back_;

  int timer_fd_{-1};
  int epoll_fd_{-1};

  std::thread work_thread_;
};

template<typename Callback>
std::shared_ptr<Timer<Callback>> create_timer(std::chrono::nanoseconds duration, Callback&& func) {
  return std::make_shared<Timer<Callback>>(duration, std::forward<Callback>(func));
}


#endif // TIME_HPP