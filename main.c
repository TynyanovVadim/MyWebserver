#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define PORT 8080

int main() {
    char resp[] = "HTTP/1.0 200 OK\r\n"
                  "Server: webserver-c\r\n"
                  "Content-type: text/html\r\n\r\n"
                  "<html>hello, world</html>\r\n";

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

    // Webserver works
    for (;;) {
        int client_sock_fd = accept(tcp_socket_fd, (struct sockaddr*)&host_addr, (socklen_t*)&host_addrlen);

        if (client_sock_fd < 0) {
            perror("webserver (accept)");
            continue;
        }

        send(client_sock_fd, resp, strlen(resp), 0);

        close(client_sock_fd);
    }

    return 0;
}