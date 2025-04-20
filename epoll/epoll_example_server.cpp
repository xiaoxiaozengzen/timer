/**
 * epoll会监控多个fd并检测是否有I/O事件发生
 * epoll中有两个list：
 * 1. interest list：注册fd的事件
 * 2. ready list：准备好进行I/O操作的fd
 */

/**
 * int epoll_create(int size);
 * 
 * @brief 创建一个epoll实例
 * 
 * @param size 自从2.6.8之后该参数在Linux内核中被忽略，但是必须大于0
 * @return 成功返回epoll实例的文件描述符，失败返回-1
 * 
 * @note 当不在需要使用epoll实例时，应该调用close()函数关闭该实例
 */

/**
 * int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
 * 
 * @brief 用于添加，修改或删除epoll中的interest list中的fd
 * 
 * @param epfd epoll_create()返回的epoll实例的文件描述符
 * @param op 操作类型: 
 *           EPOLL_CTL_ADD：添加fd到interest list，
 *           EPOLL_CTL_MOD：修改interest list中的fd，
 *           EPOLL_CTL_DEL：删除interest list中的fd
 * @param fd 需要添加，修改或删除的文件描述符
 * @param event 用于描述fd相关属性
 *              event.events：fd的事件类型，可以通过OR操作组合多个事件
 *                            EPOLLIN：可读事件
 *                            EPOLLOUT：可写事件
 *                            EPOLLRDHUP：对端关闭连接
 *                            EPOLLPRI：高优先级数据到达
 *                            EPOLLERR：错误事件
 *               event.data：内核会缓存的数据 
 */

/**
 * epoll_event：内核在fd准备好进行I/O操作时，内核会将相关的data填充到该结构体中
 * 
 * struct epoll_event {
 *     uint32_t events; // 事件类型
 *     epoll_data_t data; // 用户数据
 * };
 * 
 * union epoll_data_t {
 *     void *ptr; // 用户数据指针
 *     int fd; // 文件描述符
 *     uint32_t u32; // 32位无符号整数
 *     uint64_t u64; // 64位无符号整数
 * };
 * typedef union epoll_data  epoll_data_t;
 */

/**
 * int epoll_wait(int epfd, struct epoll_event events[.maxevents], int maxevents, int timeout);
 * 
 * @brief 等待epoll实例中的fd准备好进行I/O操作
 * 
 * @param epfd epoll_create()返回的epoll实例的文件描述符
 * @param events 用于存储准备好进行I/O操作的fd的相关信息。maxevents指定该数组的大小
 * @param maxevents 数组的大小，不超过maxevents的事件会被返回
 * @param timeout 超时时间(即epoll_wait会被阻塞的时间)，单位为毫秒
 *                如果timeout为-1，则epoll_wait会一直阻塞直到有事件发生
 *                如果timeout为0，则epoll_wait不会阻塞，立即返回
 */


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <string>
#include <iostream>

// socket 最大连接数
#define SOCKET_COUNT (64)

// epoll 监听的描述符的最大数量
#define EPOLL_COUNT (100)

int main() {

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket error");
        exit(1);
    }

    // 设置端口复用
    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 监听本地所有IP
    int ret = bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr));
    if (ret == -1) {
        perror("bind error");
        exit(1);
    }

    ret = listen(socket_fd, SOCKET_COUNT);
    if (ret == -1) {
        perror("listen error");
        exit(1);
    }

    int epoll_fd = epoll_create(EPOLL_COUNT);
    if (epoll_fd == -1) {
        perror("epoll_create error");
        exit(1);
    }

    struct epoll_event ev;
    ev.data.fd = socket_fd;
    ev.events = EPOLLIN;  // 检测 socket_fd 读缓冲区是否有数据
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev);
    if (ret == -1) {
        perror("epoll_ctl error");
        exit(1);
    }

    struct epoll_event evs[1024];
    int size = sizeof(evs) / sizeof(struct epoll_event);

    // 持续检测
    for (;;) {
        printf("epoll_wait ...\n");
        int num = epoll_wait(epoll_fd, evs, size, -1);
        for (int i = 0; i < num; ++i) {
            int cur_fd = evs[i].data.fd;
            if (cur_fd == socket_fd) {
                int new_fd = accept(socket_fd, NULL, NULL);
                ev.data.fd = new_fd;
                ev.events = EPOLLIN;
                ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, &ev);
                if (ret == -1) {
                    perror("epoll_ctl accept error");
                    continue;
                }
            } else {
                char buf[1024]{};
                int len = recv(cur_fd, buf, sizeof(buf), 0);
                if (len == 0) {
                    printf("客户端 %d 已经断开了连接\n", cur_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cur_fd, NULL);
                    close(cur_fd);
                } else if (len > 0) {
                    printf("客户端说: %s\n", buf);
                    std::string text;
                    getline(std::cin, text);  // 等待输入
                    text.insert(0, "from server: ");
                    text.push_back('\n');
                    send(cur_fd, text.c_str(), text.length(), 0);
                } else {
                    perror("socket recv error");
                    continue;
                }
            }
        }
    }

    // 关闭 socket
    close(epoll_fd);
    close(socket_fd);

    return 0;
}
