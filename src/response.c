#include "server.h"

// Function to send an HTTP response to the client
void send_response(int socket_id, const char *header, const char *body) {
    if (write(socket_id, header, strlen(header)) == -1) {
        perror("Failed to send response header");
    }
    if (body != NULL && write(socket_id, body, strlen(body)) == -1) {
        perror("Failed to send response body");
    }
}

// Function to determine the content type based on the file extension and send the file as a response
void send_file_response(int socket_id, const char *client_ip, const char *method, const char *path, const char *full_path) {
    // Check if the requested path is safe
    if (!is_safe_path(web_root, full_path)) {
        log_message("Attempted file inclusion attack detected");
        send_response(socket_id, HTTP_403, NULL);
        log_access(client_ip, method, path, 403);
        return;
    }

    // Open the file for reading
    int file_fd = open(full_path, O_RDONLY);
    if (file_fd < 0) {
        perror("open");
        log_message("Failed to open file for response");
        send_response(socket_id, HTTP_404, NULL);
        log_access(client_ip, method, path, 404);
        return;
    }

    struct stat file_stat;
    // Get the file stats to determine the content length
    if (fstat(file_fd, &file_stat) < 0) {
        perror("fstat");
        log_message("Failed to get file stats for response");
        send_response(socket_id, HTTP_404, NULL);
        close(file_fd);
        log_access(client_ip, method, path, 404);
        return;
    }

    // Get the content type based on the file extension
    const char *content_type = get_content_type(full_path);
    char response_header[BUFSIZE];
    snprintf(response_header, sizeof(response_header), "HTTP/1.0 200 OK\r\nContent-Type: %s; charset=UTF-8\r\nContent-Length: %lld\r\n\r\n", content_type, (long long)file_stat.st_size);
    send_response(socket_id, response_header, NULL);

    // Read the file content and send it to the client
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
    log_access(client_ip, method, path, 200);
}

// Function to send a directory listing as an HTML response
void send_directory_listing(int socket_id, const char *client_ip, const char *method, const char *path) {
    char response_body[BUFSIZE * 4];
    snprintf(response_body, sizeof(response_body), "<html><body><h1>Index of %s</h1><ul>", path);

    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", web_root, path);

    // Check if the requested path is safe
    if (!is_safe_path(web_root, full_path)) {
        log_message("Attempted directory traversal attack detected");
        send_response(socket_id, HTTP_403, NULL);
        log_access(client_ip, method, path, 403);
        return;
    }

    DIR *dir = opendir(full_path);
    if (dir == NULL) {
        perror("opendir");
        log_message("Failed to open directory for listing");
        send_response(socket_id, HTTP_404, NULL);
        log_access(client_ip, method, path, 404);
        return;
    }

    struct dirent *entry;
    // Iterate over the directory entries and generate the HTML response
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            strncat(response_body, "<li><a href=\"", sizeof(response_body) - strlen(response_body) - 1);
            strncat(response_body, path, sizeof(response_body) - strlen(response_body) - 1);
            strncat(response_body, "/", sizeof(response_body) - strlen(response_body) - 1);
            strncat(response_body, entry->d_name, sizeof(response_body) - strlen(response_body) - 1);
            strncat(response_body, "\">", sizeof(response_body) - strlen(response_body) - 1);
            strncat(response_body, entry->d_name, sizeof(response_body) - strlen(response_body) - 1);
            strncat(response_body, "</a></li>", sizeof(response_body) - strlen(response_body) - 1);
        }
    }
    closedir(dir);

    strncat(response_body, "</ul></body></html>", sizeof(response_body) - strlen(response_body) - 1);

    // Send the directory listing as an HTML response
    char response_header[BUFSIZE];
    snprintf(response_header, sizeof(response_header), "HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: %ld\r\n\r\n", strlen(response_body));
    send_response(socket_id, response_header, response_body);
    log_message("Directory listing response sent successfully");
    log_access(client_ip, method, path, 200);
}

void handle_request(int socket_id, const char *client_ip, const char *method, const char *path) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", web_root, path);
    struct stat path_stat;

    if (stat(full_path, &path_stat) == 0) {
        char log[512];
        snprintf(log, sizeof(log), "Handling request for %s", full_path);
        log_message(log);
        if (S_ISDIR(path_stat.st_mode)) {
            send_directory_listing(socket_id, client_ip, method, path);
        } else if (S_ISREG(path_stat.st_mode)) {
            send_file_response(socket_id, client_ip, method, path, full_path);
        } else {
            log_message("Unsupported file type");
            send_response(socket_id, HTTP_403, NULL);
            log_access(client_ip, method, path, 403);
        }
    } else {
        perror("stat");
        log_message("Failed to stat requested path");
        send_response(socket_id, HTTP_404, NULL);
        log_access(client_ip, method, path, 404);
    }
}
