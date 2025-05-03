#ifndef TIME_HPP
#define TIME_HPP

#include <sys/timerfd.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <type_traits>
#include <utility>
#include <atomic>
#include <memory>
#include <unordered_map>

using VoidCallbackType = std::function<void ()>;

template<
  typename FunT,
  typename std::enable_if<std::is_same<FunT, VoidCallbackType>::value>::type* = nullptr,
  typename Clock = std::chrono::system_clock
>
class Timer {
public:
  struct TimerCallback {
    std::chrono::nanoseconds period{0};
    FunT call_back;
    int timer_fd{-1};
  };

public:
  Timer() {
    start();
  }

  ~Timer() {
    eventfd_write(exit_fd_, 1);

    for (auto& it : timer_map_) {
      close(it.first);
    }
    if(epoll_fd_ != -1) {
      close(epoll_fd_);
    }
    if(exit_fd_ != -1) {
      close(exit_fd_);
    }
    if(work_thread_.joinable()) {
      work_thread_.join();
    }
  }

  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;
  Timer(Timer&&) = delete;
  Timer& operator=(Timer&&) = delete;

  void start() {
    std::cerr << "Timer started" << std::endl;
    exit_fd_ = eventfd(0, 0);
    if(exit_fd_ == -1) {
      throw std::runtime_error("eventfd failed");
    }

    epoll_fd_ = epoll_create(1);
    if(epoll_fd_ == -1) {
      throw std::runtime_error("epoll_create1 failed");
    }
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = exit_fd_;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, exit_fd_, &ev);
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
      if(ev.data.fd == exit_fd_) {
        uint64_t num;
        ret = eventfd_read(exit_fd_, &num);
        if(ret == -1) {
          throw std::runtime_error("read failed");
        }
        std::cerr << "Exit signal received" << std::endl;
        break;
      }
      if(timer_map_.find(ev.data.fd) != timer_map_.end()) {
        uint64_t num;
        ret = read(ev.data.fd, &num, sizeof(num));
        if(ret == -1) {
          throw std::runtime_error("read failed");
        }

        execute_callback(ev.data.fd);
      }
    }
  }

  void AddCallback(std::chrono::nanoseconds period, FunT&& func) {
    std::cerr << "AddCallback called" << std::endl;
    if(period <= std::chrono::nanoseconds(0)) {
      throw std::runtime_error("period must be greater than 0");
    }
  
    int timer_fd_ = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    if(timer_fd_ == -1) {
      throw std::runtime_error("timerfd_create failed");
    }
  
    struct itimerspec duration;
    duration.it_interval.tv_sec = period.count() / 1000000000;
    duration.it_interval.tv_nsec = period.count() % 1000000000;
    duration.it_value.tv_sec = period.count() / 1000000000;
    duration.it_value.tv_nsec = period.count() % 1000000000;
  
    int ret = timerfd_settime(timer_fd_, 0, &duration, nullptr);
    if(ret == -1) {
      throw std::runtime_error("timerfd_settime failed");
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = timer_fd_;
    ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, timer_fd_, &ev);
    if(ret == -1) {
      throw std::runtime_error("epoll_ctl failed");
    }

    timer_map_[timer_fd_] = TimerCallback{period, std::forward<FunT>(func), timer_fd_};
    std::cerr << "Callback added" << std::endl;
  }

  void execute_callback(int fd) {
    execute_call_back<>(fd);
  }

  template<
    typename CallBack = FunT,
    typename std::enable_if<std::is_same<FunT, VoidCallbackType>::value>::type* = nullptr
  >
  void execute_call_back(int fd) {
    if(timer_map_.find(fd) != timer_map_.end()) {
      timer_map_.at(fd).call_back();
    }
  }

private:
  int epoll_fd_{-1};
  int exit_fd_{-1};

  std::unordered_map<int, TimerCallback> timer_map_{};

  std::thread work_thread_;
};

template<typename Callback>
std::shared_ptr<Timer<Callback>> create_timer() {
  return std::make_shared<Timer<Callback>>();
}


#endif // TIME_HPP