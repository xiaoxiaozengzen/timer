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
  std::cout << ss.str() << "fun1 called" << std::endl;
}

void fun2(int a) {
  std::cout << "fun2 called" << std::endl;
}

int main() {
  auto timer = create_timer<std::function<void()>>(std::chrono::nanoseconds(1000000000), fun1);
  
  while(true) {
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 延迟 1 秒
  }

  return 0;
}