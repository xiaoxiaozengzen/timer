#include <sys/eventfd.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main() {

  /**
   * @brief Create an eventfd object
   * 
   * @param initval Initial value of the eventfd object
   * @param flags Flags for the eventfd object
   * 
   * @note 在2.6.26之前版本，没有flag，因此必须强制使用0
   * @note 在2.6.26版本之后，eventfd()函数可以使用flags参数来指定事件的类型
   * @note EFD_CLOEXEC: 关闭exec时关闭eventfd对象
   * @note EFD_NONBLOCK: 非阻塞模式
   */
  int fd = eventfd(0, EFD_NONBLOCK);
  if(fd == -1) {
    printf("eventfd failed\n");
    return -1;
  }

  /**
   * @brief Write to the eventfd object
   * 
   * @param fd File descriptor of the eventfd object
   * @param value Value to write to the eventfd object
   * 
   * @note eventfd_write()函数用于向eventfd对象写入一个64位的无符号整数, 增加事件的计数器
   */
  eventfd_t write_value = 2;
  int ret = eventfd_write(fd, write_value);
  if(ret == -1) {
    printf("eventfd_write failed\n");
    return -1;
  } else {
    printf("eventfd_write ret %d\n", ret);
  }

  /**
   * @brief Read from the eventfd object
   * 
   * @param fd File descriptor of the eventfd object
   * @param value Pointer to store the read value
   * 
   * @note eventfd_read()函数用于从eventfd对象中读取一个64位的无符号整数, 并重设置计数器
   * @note 如果计数器的值为0，则会阻塞直到有事件发生。可以在创建eventfd对象时使用EFD_NONBLOCK标志来避免阻塞
   */
  eventfd_t value;
  ret = eventfd_read(fd, &value);
  if(ret == -1) {
    printf("eventfd_read failed\n");
    return -1;
  } else {
    printf("eventfd_read ret %ld\n", value);
  }

  eventfd_write(fd, 1);
  ret = eventfd_read(fd, &value);
  if(ret == -1) {
    printf("eventfd_read failed\n");
    return -1;
  } else {
    printf("eventfd_read ret %ld\n", value);
  }


  return 0;
}