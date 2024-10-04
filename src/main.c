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

#define BUFSIZE 4096
#define DEFAULT_PORT 8080
#define DEFAULT_ROOT "./www"
#define CONFIG_FILE_1 "/etc/mini_httpd.conf"
#define CONFIG_FILE_2 "./etc/mini_httpd.conf"
#define LOG_FILE "./var/log/server.log"

char server_ip[16] = "0.0.0.0";
int server_port = DEFAULT_PORT;
char web_root[256] = DEFAULT_ROOT;
char log_file_path[256] = LOG_FILE;
void log_message(const char *message) {
    FILE *log_file = fopen(log_file_path, "a");
    if (log_file) {
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        timestamp[strlen(timestamp) - 1] = '\0';  // Remove newline
        fprintf(log_file, "%s: %s\n", timestamp, message);
        fclose(log_file);
    }
}

void load_config() {
    FILE *config_file = fopen(CONFIG_FILE_1, "r");
    if (config_file == NULL) {
        config_file = fopen(CONFIG_FILE_2, "r");
    }
    if (config_file) {
        char line[256];
        while (fgets(line, sizeof(line), config_file)) {
            if (sscanf(line, "server_ip = %15s", server_ip) == 1) continue;
            if (sscanf(line, "server_port = %d", &server_port) == 1) continue;
            if (sscanf(line, "server_root = %255s", web_root) == 1) continue;
            if (sscanf(line, "log_file = %255s", log_file_path) == 1) continue;
        }
        fclose(config_file);
    }
    char log[256];
    snprintf(log, sizeof(log), "Configuration loaded: IP=%s, Port=%d, Root=%s", server_ip, server_port, web_root);
    log_message(log);
}

void parse_request(const char *buffer, char *method, char *path) {
    sscanf(buffer, "%15s %255s", method, path);
    if (strcmp(path, "/") == 0) {
        strcpy(path, "/index.html");
    }
}

void send_response(int socket_id, const char *header, const char *body) {
    write(socket_id, header, strlen(header));
    if (body != NULL) {
        write(socket_id, body, strlen(body));
    }
}

const char* get_content_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (ext) {
        if (strcmp(ext, ".html") == 0) return "text/html";
        if (strcmp(ext, ".css") == 0) return "text/css";
        if (strcmp(ext, ".js") == 0) return "application/javascript";
        if (strcmp(ext, ".png") == 0) return "image/png";
        if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
        if (strcmp(ext, ".gif") == 0) return "image/gif";
        if (strcmp(ext, ".txt") == 0) return "text/plain";
    }
    return "application/octet-stream";
}

void send_file_response(int socket_id, const char *full_path) {
    int file_fd = open(full_path, O_RDONLY);
    if (file_fd < 0) {
        perror("open");
        log_message("Failed to open file for response");
        send_response(socket_id, "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\n404 Not Found", NULL);
        return;
    }

    struct stat file_stat;
    if (fstat(file_fd, &file_stat) < 0) {
        perror("fstat");
        log_message("Failed to get file stats for response");
        send_response(socket_id, "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\n404 Not Found", NULL);
        close(file_fd);
        return;
    }

    const char *content_type = get_content_type(full_path);
    char response_header[BUFSIZE];
    snprintf(response_header, sizeof(response_header), "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %lld\r\n\r\n", content_type, (long long)file_stat.st_size);
    send_response(socket_id, response_header, NULL);

    char file_buffer[BUFSIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, file_buffer, BUFSIZE)) > 0) {
        if (write(socket_id, file_buffer, bytes_read) != bytes_read) {
            perror("write");
            log_message("Failed to write file content to socket");
            break;
        }
    }
    if (bytes_read < 0) {
        perror("read");
        log_message("Failed to read file for response");
    }
    close(file_fd);
    log_message("File response sent successfully");
}

void send_directory_listing(int socket_id, const char *path) {
    char response_body[BUFSIZE * 4];
    snprintf(response_body, sizeof(response_body), "<html><body><h1>Index of %s</h1><ul>", path);

    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", web_root, path);

    DIR *dir = opendir(full_path);
    if (dir == NULL) {
        perror("opendir");
        log_message("Failed to open directory for listing");
        send_response(socket_id, "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\n404 Not Found", NULL);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(response_body + strlen(response_body), sizeof(response_body) - strlen(response_body),
                     "<li><a href=\"%s/%s\">%s</a></li>", path, entry->d_name, entry->d_name);
        }
    }
    closedir(dir);

    strcat(response_body, "</ul></body></html>");

    char response_header[BUFSIZE];
    snprintf(response_header, sizeof(response_header), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", strlen(response_body));
    send_response(socket_id, response_header, response_body);
    log_message("Directory listing response sent successfully");
}

void handle_request(int socket_id, const char *method, const char *path) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", web_root, path);
    struct stat path_stat;

    if (stat(full_path, &path_stat) == 0) {
        char log[512];
        snprintf(log, sizeof(log), "Handling request for %s", full_path);
        log_message(log);
        if (S_ISDIR(path_stat.st_mode)) {
            send_directory_listing(socket_id, path);
        } else if (S_ISREG(path_stat.st_mode)) {
            send_file_response(socket_id, full_path);
        } else {
            log_message("Unsupported file type");
            send_response(socket_id, "HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\n403 Forbidden", NULL);
        }
    } else {
        perror("stat");
        log_message("Failed to stat requested path");
        send_response(socket_id, "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\n404 Not Found", NULL);
    }
}

int create_server_socket() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        log_message("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        log_message("Failed to set socket options");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

void bind_and_listen(int server_fd) {
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(server_ip);
    address.sin_port = htons(server_port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        log_message("Failed to bind server socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        log_message("Failed to listen on server socket");
        exit(EXIT_FAILURE);
    }

    char log[256];
    snprintf(log, sizeof(log), "Server listening on %s:%d", server_ip, server_port);
    log_message(log);
    printf("Server listening on %s:%d...\n", server_ip, server_port);
}

void *handle_client(void *arg) {
    int socket_id = *(int *)arg;
    free(arg);

    char buffer[BUFSIZE];
    char method[16];
    char path[256];

    memset(buffer, 0, BUFSIZE);
    read(socket_id, buffer, BUFSIZE);
    printf("Request received:\n%s\n", buffer);

    parse_request(buffer, method, path);
    printf("Method: %s, Path: %s\n", method, path);

    if (strcmp(method, "GET") == 0) {
        handle_request(socket_id, method, path);
    } else {
        log_message("Unsupported HTTP method");
        send_response(socket_id, "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\n501 Not Implemented", NULL);
    }
    close(socket_id);
    return NULL;
}

int main() {
    load_config();

    int server_fd = create_server_socket();
    bind_and_listen(server_fd);

    struct sockaddr_in address;
    int addrlen = sizeof(address);

    while (1) {
        int *socket_id = malloc(sizeof(int));
        *socket_id = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (*socket_id < 0) {
            perror("accept");
            log_message("Failed to accept client connection");
            free(socket_id);
            continue;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, socket_id) != 0) {
            perror("pthread_create");
            log_message("Failed to create thread for client");
            free(socket_id);
        } else {
            pthread_detach(thread_id);
        }
    }

    close(server_fd);
    return 0;
}
