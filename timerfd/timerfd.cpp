/**
 * timerfd的特点是将时间变成一个文件描述符，定时器超时时，文件可读。
 * 这样就能很容易融入select(2)/poll(2)/epoll(7)的框架中，用统一的方式来处理IO事件、超时事件。
 */

/**
 * int timerfd_create(int clockid, int flags);
 * 
 * @brief 创建定时器对象，并返回一个文件描述符
 * 
 * @param clockid 定时器类型，只能是CLOCK_REALTIME或CLOCK_MONOTONIC
 * @param flags 定时器属性，值能按位或：
 *            1）TFD_NONBLOCK：为新打开的fd设置O_NONBLOCK选项   
 *            2）TFD_CLOEXEC：为新打开的fd设置FD_CLOEXEC选项，即fork+exec时关闭fd
 */

/**
 * int timerfd_settime(int fd, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);
 * 
 * @brief 用于启动或停止绑定到fd的定时器
 * 
 * @param fd timerfd_create()返回的文件描述符
 * @param flags 定时器属性: 1) 0表示相对定时器，基于当前时间+new_value的值 2) TFD_TIMER_ABSTIME表示绝对定时器，基于new_value的值
 * @param new_value 定时器的时间值
 * @param old_value 旧的定时器值
 * 
 * @note read: 定时器超时后，fd可读，返回值为定时器超时的次数。如果没有超时，read会阻塞
 * @note poll/select/epoll：监听fd可读事件，fd可读时，会受到就绪通知
 * @note close：关闭fd时，定时器会停止  
 */

/**
 * int timerfd_gettime(int fd, struct itimerspec *curr_value);
 * 
 * @brief 获取定时器的当前值
 * 
 * @param fd timerfd_create()返回的文件描述符
 * @param curr_value 定时器的当前值
 */

/**
 * struct itimerspec {
 *   struct timespec it_interval; // 定时器的间隔时间
 *   struct timespec it_value; // 定时器第一次超时的时间
 * };
 * 
 * struct timespec {
 *   time_t tv_sec; // 秒
 *   long tv_nsec; // 纳秒
 * };
 */

#include <iostream>
#include <sys/timerfd.h>
#include <unistd.h>
#include <ctime>
#include <poll.h>
#include <cerrno>
#include <chrono>
#include <thread>

int main() {
  struct itimerspec timebuf;
  int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  timebuf.it_interval = {1, 0}; // period timeout value = 1s
  timebuf.it_value = {5, 100};  // initial timeout value = 5s100ns
  timerfd_settime(timerfd, 0, &timebuf, NULL);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::seconds(2)); // 延迟 2 秒
    uint64_t num;
    int ret = read(timerfd, &num, sizeof(num));
    if (ret == -1) {
      if(errno != EAGAIN || errno != EWOULDBLOCK) {
        std::cerr << "read error: " << errno << std::endl;
        break;  
      } 
      continue;
    }
    std::cout << "timerfd read ret: " << ret << ", num: " << num << std::endl;
  }
  close(timerfd);
  return 0;
}