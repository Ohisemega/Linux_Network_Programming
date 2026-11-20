#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <assert.h>

static void do_something(int connfd);
static int32_t read_full(int fd, char* rbuff, size_t n, int flags = 0);
static int32_t write_all(int fd, const char* wbuff, size_t n, int flags = 0);

void server() {
    int fd = socket(AF_INET, SOCK_STREAM, 0); // a socket stream is a socket that
                                                                      // accepts streaming data over TCP
    int val = 1;
    // The setsockopt() call is used to configure various aspects of a socket.
    // This particular call enables the SO_REUSEADDR option. Without this option, 
    // the server won’t able to bind to the same address if restarted
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = ntohs(1234), .sin_addr{.s_addr = ntohl(0)}};
    int rv = bind(fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
    
    // listen
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        std::cout << "bind() failed: " << rv << "!\n";
        exit(1);
    }

    while(true) {
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);
        int connfd = accept(fd, reinterpret_cast<struct sockaddr*>(&client_addr), &socklen);
        if ((connfd < 0))
            continue;
        if (socklen > sizeof(client_addr))
            std::cout << "socklen exceeded provided size: " << socklen << " vs " << sizeof(client_addr) << '\n';
        // do_something(connfd);
        // only serves one connection at once
        while (true) {
            int32_t err = one_request(connfd);
            if (err) break;
        }
        close(connfd);
    }
}

int32_t read_full(int fd, char* buf, size_t n, int flags) {
    while (n > 0) {
        ssize_t bytes_recv = recv(fd, buf, n, flags);
        if (bytes_recv <= 0) return -1;
        assert(static_cast<size_t>(bytes_recv) <= n);
        n -= static_cast<size_t>(bytes_recv);
        buf += bytes_recv;
    }
    return 0;
}

int32_t write_all(int fd, const char* buf, size_t n, int flags) {
    while (n > 0) {
        ssize_t bytes_recv = send(fd, buf, n, 0);
        if (bytes_recv <= 0) return -1;
        assert(static_cast<size_t>(bytes_recv) <= n);
        n -= static_cast<size_t>(bytes_recv);
        buf += bytes_recv;
    }
    return 0;
}

constexpr size_t k_max_msg = 4096;
static int32_t one_request(int connfd) {
    // 4 bytes header
    char rbuf[4+k_max_msg+1];
    // errno = 0;
    int32_t err = read_full(connfd, rbuf, 4);
    if(err) {
        if (errno == 0) std::cout << "EOF!\n";
        else std::cout << "read() failed: erron(" << errno << ")\n";
        return err;
    }
    uint32_t len = 0;
    memcpy(&len, rbuf, 4); // assume litle endian
    if (len > k_max_msg) {
        std::cout << "too long\n";
        return -1;
    }
    // request body;
    err = read_full(connfd, &rbuf[4], len);
    if (err) {
        std::cout << "read() error\n";
        return err;
    }
    // do something
    static int msg_cnt = 0;
    rbuf[4+len] = '\0';
    std::cout << "Client says: " << &rbuf[4] << '\n';
    char reply[16];
    snprintf(reply, sizeof(reply), "Nice msg: %d", msg_cnt);
    ++msg_cnt;
    return send(connfd, reply, strlen(reply), 0);
}

int main() {
    server();
}
