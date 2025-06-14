#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <cstdint>

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
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int ret = connect(socket_fd, (struct sockaddr *) &addr, sizeof(addr));
    if (ret == -1) {
        perror("connect error");
        exit(1);
    }

    char buf[1024]{};
    uint64_t counter = 0;
    while(true) {
        counter++;
        sprintf(buf, "Hello from client! Counter: %lu", counter);
        int len = send(socket_fd, buf, strlen(buf), 0);
        if (len < 0) {
            perror("send error");
            exit(1);
        }

        len = recv(socket_fd, buf, sizeof(buf), 0);
        if (len < 0) {
            perror("recv error");
            exit(1);
        }
        printf("client recv: %s\n", buf);
        sleep(1);  // 每秒发送一次数据       
    }

    // 关闭 socket
    close(socket_fd);

    return 0;
}
