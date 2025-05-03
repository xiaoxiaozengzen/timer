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
  std::cerr << ss.str() << "fun1 called" << std::endl;
}

void fun2() {
  std::stringstream ss;
  uint64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  ss << "[" << std::to_string(timestamp/1000000000) << "." << std::to_string(timestamp%1000000000) << "]: ";
  std::cerr << ss.str() << "fun2 called" << std::endl;
}

int main() {
  auto timer = create_timer<std::function<void()>>();

  timer->AddCallback(std::chrono::nanoseconds(1000000000), fun1);
  timer->AddCallback(std::chrono::nanoseconds(2000000000), fun2);
  
  while(true) {
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 延迟 1 秒
  }

  return 0;
}