#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

#include "webserver.h"

int main() {
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
    int id_to_fd[SOMAXCONN + 1] = {tcp_socket_fd};
    struct pollfd poll_idx[SOMAXCONN + 1];

    poll_idx[0].fd = tcp_socket_fd;
    poll_idx[0].events = POLLIN;

    for (int i = 1; i <= SOMAXCONN + 1; ++i) {
        poll_idx[i].fd = -1;
        id_to_fd[i] = -1;
    }

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

        if (poll_idx[0].revents & POLLIN) {
            poll_idx[0].revents = 0;
            int client_sock_fd = accept(tcp_socket_fd, (struct sockaddr*)&host_addr, (socklen_t*)&host_addrlen);

            if (client_sock_fd < 0) {
                perror("webserver (accept)");
                continue;
            }
            
            for (int i = 1; i <= SOMAXCONN + 1; ++i) {
                if (poll_idx[i].fd == -1) {
                    poll_idx[i].fd = client_sock_fd;
                    poll_idx[i].events = POLLIN;
                    id_to_fd[i] = client_sock_fd;
                    break;
                }
            }
            continue;
        }

        for (int i = 1; i <= SOMAXCONN + 1; ++i) {
            if (poll_idx[i].revents & POLLIN) {
                poll_idx[i].revents = 0;
                if (talk(id_to_fd[i]) == 0) {
                    close(id_to_fd[i]);
                    poll_idx[i].fd = -1;
                    id_to_fd[i] = -1;    
                }
            }
        }
    }

    return 0;
}

int talk(int client_sock_fd) {
    char buf[10000] = {};
    char resp[] = "HTTP/1.0 200 OK\r\n"
                  "Server: webserver-c\r\n"
                  "Content-type: text/html\r\n\r\n";

    int recv_bytes = recv(client_sock_fd, buf, 10000, 0);
    // printf("%s\n", buf);
    snprintf(buf, 10000, "%s<html>%d</html>\r\n", resp, client_sock_fd);
    send(client_sock_fd, buf, strlen(buf), 0);
    return recv_bytes;
}