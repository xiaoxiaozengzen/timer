#include <iostream>

#include "time.hpp"

void fun1() {
  std::cout << "fun1 called" << std::endl;
}

void fun2(int a) {
  std::cout << "fun2 called" << std::endl;
}

int main() {
  auto timer = create_timer<std::function<void()>>(std::chrono::nanoseconds(1000), fun1);
  timer->execute_callback();

  return 0;
}