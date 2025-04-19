#ifndef TIME_HPP
#define TIME_HPP

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
    if(period_.count() <= 0) {
      std::cerr << "Period must be no-negative" << std::endl;
      return;
    }

    if(started_) {
      std::cout << "Timer already started" << std::endl;
      return;
    }
    
    std::cout << "Timer started" << std::endl;
    started_.store(true);

    start_time_ = Clock::now();
    last_call_time_ = start_time_;
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
  }

private:
  typename Clock::time_point start_time_;
  typename Clock::time_point last_call_time_;
  typename Clock::time_point next_call_time_;
  
  

  std::atomic<bool> started_{false};
  std::atomic<bool> stopped_{false};

  std::chrono::nanoseconds period_;
  FunT call_back_;
};

template<typename Callback>
std::shared_ptr<Timer<Callback>> create_timer(std::chrono::nanoseconds duration, Callback&& func) {
  return std::make_shared<Timer<Callback>>(duration, std::forward<Callback>(func));
}


#endif // TIME_HPP