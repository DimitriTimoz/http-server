#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#define BUFSIZE 4096
#define DEFAULT_PORT 8080
#define DEFAULT_ROOT "./www"
#define CONFIG_FILE_1 "/etc/mini_httpd.conf"
#define CONFIG_FILE_2 "./etc/mini_httpd.conf"
#define LOG_FILE "./var/log/server.log"
#define ACCESS_LOG_FILE "./var/log/access.log"

#define HTTP_403 "HTTP/1.0 403 Forbidden\r\nContent-Type: text/plain; charset=UTF-8\r\nContent-Length: 13\r\n\r\n403 Forbidden"
#define HTTP_404 "HTTP/1.0 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\n404 Not Found"
#define HTTP_501 "HTTP/1.0 501 Not Implemented\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\n501 Not Implemented"
#define HTTP_405 "HTTP/1.0 405 Method Not Allowed\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\n405 Method Not Allowed"

extern char server_ip[16];
extern int server_port;
extern char web_root[256];
extern char log_file_path[256];

void log_message(const char *message);
void log_access(const char *client_ip, const char *method, const char *path, int status_code);
void load_config();
void parse_request(const char *buffer, char *method, size_t method_size, char *path, size_t path_size);
void send_response(int socket_id, const char *header, const char *body);
const char* get_content_type(const char *path);
int is_safe_path(const char *base_path, const char *requested_path);
void send_file_response(int socket_id, const char *client_ip, const char *method, const char *path, const char *full_path);
void send_directory_listing(int socket_id, const char *client_ip, const char *method, const char *path);
void handle_request(int socket_id, const char *client_ip, const char *method, const char *path);
int create_server_socket();
void bind_and_listen(int server_fd);
void *handle_client(void *arg);

#endif
