// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <errno.h>
// #include <stdio.h>
// #include <string.h>
// #include <poll.h>
// #include <signal.h>

#include "webserver.h"

int main() {
    signal(SIGPIPE, SIG_IGN);

    int tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket_fd == -1) {
        perror("webserver (socket)");
        return 1;
    }

    // Create the address to bind socket to
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket to the address
    if (bind(tcp_socket_fd, (struct sockaddr*)&host_addr, host_addrlen) != 0) {
        perror("webserver (bind)");
        return 1;
    }

    // Listen for incomming connection
    if (listen(tcp_socket_fd, SOMAXCONN) != 0) {
        perror("webserver (listen)");
        return 1;
    }

    // Create polls
    struct pollfd poll_idx[SOMAXCONN + 1];

    
    for (int i = 1; i <= SOMAXCONN + 1; ++i) {
        poll_idx[i].fd = -1;
    }

    poll_idx[tcp_socket_fd].fd = tcp_socket_fd;
    poll_idx[tcp_socket_fd].events = POLLIN;

    // Webserver works
    for (;;) {
        int poll_res = poll(poll_idx, SOMAXCONN + 1, 100);

        if (poll_res == -1) {
            perror("webserver (poll)");
            return 1;
        } else if (poll_res == 0) {
            /* timeout */
            continue;
        } 

        if (poll_idx[tcp_socket_fd].revents & POLLIN) {
            poll_idx[tcp_socket_fd].revents = 0;
            int client_sock_fd = accept(tcp_socket_fd, (struct sockaddr*)&host_addr, (socklen_t*)&host_addrlen);

            if (client_sock_fd < 0) {
                perror("webserver (accept)");
                continue;
            }
            
            poll_idx[client_sock_fd].fd = client_sock_fd;
            poll_idx[client_sock_fd].events = POLLIN;
        }

        for (int i = 0; i <= SOMAXCONN + 1; ++i) {
            if (i != tcp_socket_fd && poll_idx[i].revents & POLLIN) {
                poll_idx[i].revents = 0;
                if (talk(i) == 0) {
                    close(i);
                    poll_idx[i].fd = -1; 
                }
            }
        }
    }

    close(tcp_socket_fd);
    return 0;
}
