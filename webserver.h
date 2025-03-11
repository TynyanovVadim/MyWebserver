#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>

#define PORT 8080
#define ROOT_DIR "/var/www/html"

typedef struct {
    char method[16];  // GET, POST и т.д.
    char path[256];   // Путь запроса (/index.html)
    char version[16]; // Версия HTTP (HTTP/1.1)
} HttpRequest;

ssize_t talk(int client_sock_fd);
int parse_http_request(const char* request, HttpRequest* req);