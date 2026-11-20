#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include "utility.h"

constexpr size_t k_msg_max = 4096;

static int32_t query (int fd, const char* text) {
    uint32_t len = static_cast<uint32_t>(strlen(text));
    if (len > k_msg_max) {
        return -1;
    }

    char wbuf[4 + k_msg_max];
    memcpy(wbuf, &len, 4);  // assume little endian
    memcpy(&wbuf[4], text, len);
    if (int32_t err = write_all(fd, wbuf, 4 + len))
        return err;

    // 4 bytes header
    char rbuf[4 + k_msg_max + 1];
    errno = 0;
    int32_t err = read_full(fd, rbuf, 4);
    if (err) {
        if (errno == 0) std::cout << "EOF!\n";
        else std::cout << "read() error\n";
        return err;
    }
    memcpy(&len, rbuf, 4);
    if (len > k_msg_max) {
        std::cout << "Reply too long "<< len << '\n';
        return -1;
    }
    // reply body
    err = read_full(fd, &rbuf[4], len);
    if (err) {
        std::cout << "read() error\n";
        return -1;
    }
    // do something
    rbuf[4+len] = '\0';
    std::cout << "Server says: " << &rbuf[4] << '\n';
    return 0;
}


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

    int32_t err = query(fd, "Hello1") | query(fd, "Hello2") | query(fd, "3rd Charm!");
    if (err) {
        close(fd);
        return;
    }

    /*char msg[] = "hello";
    write(fd, msg, strlen(msg));

    char rbuf[64] = {};
    ssize_t n = recv(fd, rbuf, sizeof(rbuf)-1, 0);
    if (n < 0) {
        std::cout << "recv() failed! " << n << " errno: " << errno <<  '\n';
        close(fd);
        exit(1);
    }
    std::cout << "Server says: " << rbuf << '\n';
    close(fd);*/
}

int main() {
    client();
}
