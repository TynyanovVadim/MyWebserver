#include "webserver.h"

ssize_t talk(int client_sock_fd) {
    char buf[BUFSIZE] = {};

    ssize_t recv_bytes = recv(client_sock_fd, buf, BUFSIZE - 1, 0);
    if (recv_bytes <= 0) return recv_bytes;

    HttpRequest req;
    HttpResponse res;

    parse_http_request(buf, &req);

    if (strcmp(req.method, "GET") != 0) {
        const char *response = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
        send(client_sock_fd, response, strlen(response), 0);
        return 1;
    }

    char tmp[PATH_MAX - 10];
    strncpy(tmp, req.path, PATH_MAX - 10);
    snprintf(req.path, PATH_MAX, ROOT_DIR "%s", tmp);

    int static_file_status = serve_static_file(&req, &res);
    ssize_t fd = open(req.path, O_RDONLY);
    if (fd < 0) {
        perror(req.path);
        res.answerCode = 404;
        strncpy(res.answerText, "Not Found", 10);
    }

    snprintf(buf, BUFSIZE, "HTTP/1.1 %d %s\r\n"
                         "Server: webserver-c\r\n"
                         "Content-type: %s\r\n" , res.answerCode, res.answerText, res.type);
    send(client_sock_fd, buf, strlen(buf), 0);

    if (static_file_status != 0 || fd <= 0) {
        const char *response = "Content-Length: 0\r\n\r\n";
        send(client_sock_fd, response, strlen(response), 0);
        return 1;
    }

    struct stat file_stat = get_file_stat(fd);
    snprintf(buf, 4096, "Content-Length: %ld\r\n\r\n", file_stat.st_size);
    send(client_sock_fd, buf, strlen(buf), 0);

    

    size_t read_doc_size = 0;
    while (read_doc_size < file_stat.st_size) {
        int read_size = read(fd, buf, BUFSIZE - 1);
        read_doc_size += read_size;
        int curr = send(client_sock_fd, buf, read_size, 0);
        if (curr <= 0) return 0;

        recv_bytes += curr;
    }


    return recv_bytes;
}

int parse_http_request(const char* request, HttpRequest* req) {
    sscanf(request, "%15s %1023s %15s", req->method, req->path, req->version);
    if (strcmp("/", req->path) == 0) {
        memcpy(req->path, DEFAULT_PATH, strlen(DEFAULT_PATH) + 1);
    }
    return 0;
}

int serve_static_file(HttpRequest* req, HttpResponse* res) {
    normalize_path(req->path);

    if (!is_path_valid(req->path) || !is_path_safe(req->path)) {
        res->answerCode = 400;
        strncpy(res->answerText, "Bad Request", 256);
        strncpy(res->type, "text/html", 256);
        perror(req->path);
        return 1;
    }

    get_response_type(req->path, res);
    res->answerCode = 200;
    strncpy(res->answerText, "OK", 256);
    return 0;
}

struct stat get_file_stat(int fd) {
    struct stat fd_stat;
    fstat(fd, &fd_stat);
    return fd_stat;
}

int is_path_valid(const char *path) {
    const char *invalid_chars = "\0:*?<>|"; // forbiden symbols
    return strpbrk(path, invalid_chars) == NULL;
}

int is_path_safe(const char *requested_path) {
    char root_dir[PATH_MAX], absolute_path[PATH_MAX];
    realpath(ROOT_DIR, root_dir);
    realpath(requested_path, absolute_path);
    // if in root_dir
    return strncmp(root_dir, absolute_path, strlen(root_dir)) == 0;
}

void normalize_path(char *path) {
    char *p = path;
    while (*p) {
        if (strncmp(p, "../", 3) == 0) {
            memmove(p, p + 3, strlen(p + 3) + 1);
        } else {
            p++;
        }
    }
}

int get_response_type(const char *path, HttpResponse *res) {
    const char* file_type = strrchr(path, '.');
    if (file_type == NULL) {
        strncpy(res->type, "text/html", 256);
        return 0;
    }
    ++file_type;

    // image types
    if (type_compare(file_type, "webp", "png", "jpeg", "gif", "")) {
        snprintf(res->type, 256, "image/%s", file_type);
        return 0;
    }
    
    // text types
    if (type_compare(file_type, "html", "css", "csv", "markdown", "javascript", "")) {
        snprintf(res->type, 256, "text/%s", file_type);
        return 0;
    }

    strncpy(res->type, "text/html", 256);
    return 0;
}

int type_compare(const char* s1, const char* s2, ...) {
    if (strcmp(s1, s2) == 0) return 1;

    va_list factor;
    va_start(factor, s2);
    for (;;) {
        const char *s = va_arg(factor, const char*);
        if (*s == '\0') break;
        if (strcmp(s1, s) == 0) return 1;
    }
    va_end(factor);

    return 0;
}
