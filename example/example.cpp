#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <sstream>

#include "time.hpp"

void fun1() {
  std::stringstream ss;
  uint64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  ss << "[" << std::to_string(timestamp/1000000000) << "." << std::to_string(timestamp%1000000000) << "]: ";
  ss << "fun1 called";
  std::cerr << ss.str() << std::endl;
}

void fun2() {
  std::stringstream ss;
  uint64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  ss << "[" << std::to_string(timestamp/1000000000) << "." << std::to_string(timestamp%1000000000) << "]: ";
  ss << "fun2 called";
  std::cerr << ss.str() << std::endl;
}

class Test {
public:
    Test() {
      timer_ = create_timer<std::function<void()>>();
      timer_->AddCallback(std::chrono::nanoseconds(3000000000), std::bind(&Test::Callback1, this));
      timer_->AddCallback(std::chrono::nanoseconds(4000000000), std::bind(&Test::Callback2, this));
    }
  
    ~Test() {

    }

    void Callback1() {
      std::stringstream ss;
      uint64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();
      ss << "[" << std::to_string(timestamp/1000000000) << "." << std::to_string(timestamp%1000000000) << "]: ";
      ss << "Test Callback1 called";
      std::cerr << ss.str() << std::endl;
    }

    void Callback2() {
      std::stringstream ss;
      uint64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count();
      ss << "[" << std::to_string(timestamp/1000000000) << "." << std::to_string(timestamp%1000000000) << "]: ";
      ss << "Test Callback2 called";
      std::cerr << ss.str() << std::endl;
    }

private:
    std::shared_ptr<Timer<std::function<void()>>> timer_{nullptr};
};

int main() {
  auto timer = create_timer<std::function<void()>>();

  timer->AddCallback(std::chrono::nanoseconds(1000000000), fun1);
  timer->AddCallback(std::chrono::nanoseconds(2000000000), fun2);
  Test test;
  
  while(true) {
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 延迟 1 秒
  }

  return 0;
}