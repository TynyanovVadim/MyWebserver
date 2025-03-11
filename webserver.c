
#include "webserver.h"

ssize_t talk(int client_sock_fd) {
    char buf[4096] = {};
    char resp[] = "HTTP/1.0 200 OK\r\n"
                  "Server: webserver-c\r\n"
                  "Content-type: text/html\r\n\r\n";

    ssize_t recv_bytes = recv(client_sock_fd, buf, 10000, 0);
    if (recv_bytes <= 0) return recv_bytes;

    HttpRequest req;
    parse_http_request(buf, &req);
    printf("%s\n%s\n%s\n", req.method, req.path, req.version);

    snprintf(buf, 10000, "%s<html>%d</html>\r\n", resp, client_sock_fd);
    send(client_sock_fd, buf, strlen(buf), 0);
    return recv_bytes;
}

int parse_http_request(const char* request, HttpRequest* req) {
    sscanf(request, "%s %255s %s", req->method, req->path, req->version);
    return 0;
}