#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdarg.h>

#define PORT 8080
#define BUFSIZE 4096
#define PATH_MAX 1024
#define ROOT_DIR "./docroot"
#define DEFAULT_PATH "/index.html"

typedef struct {
    char method[16];  // GET, POST и т.д.
    char path[PATH_MAX];   // Путь запроса (/index.html)
    char version[16]; // Версия HTTP (HTTP/1.1)
} HttpRequest;

typedef struct {
    short answerCode;
    char answerText[256];  // 200, 300, 404 etc.
    char type[256];   // text/html, png etc.
} HttpResponse;

ssize_t talk(int client_sock_fd);
int serve_static_file(HttpRequest* req, HttpResponse* res);
int parse_http_request(const char* request, HttpRequest* req);
struct stat get_file_stat(int fd);

void normalize_path(char *path);
int is_path_valid(const char *path);
int is_path_safe(const char *requested_path);
int get_response_type(const char *path, HttpResponse *res);
int type_compare(const char* s1, const char* s2, ...);