#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

void client() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cout << "socket() failed!\n";
        exit(1);
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);  // 127.0.0.1
    int rv = connect(fd, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr));
    if (rv) {
        std::cout << "connect() failed!\n";
        exit(1);
    }

    char msg[] = "hello";
    write(fd, msg, strlen(msg));

    char rbuf[64] = {};
    ssize_t n = recv(fd, rbuf, sizeof(rbuf)-1, 0);
    if (n < 0) {
        std::cout << "recv() failed! " << n << " errno: " << errno <<  '\n';       
        exit(1);
    }
    std::cout << "Server says: " << rbuf << '\n';
    close(fd);
}

int main() {
    client();
}
