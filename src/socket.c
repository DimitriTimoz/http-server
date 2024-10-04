#include "server.h"

// Function to create a server socket
int create_server_socket() {
    // Create a new socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        log_message("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    // Set the socket options to allow the server to be restarted immediately after termination
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        log_message("Failed to set socket options");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

void bind_and_listen(int server_fd) {
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(server_ip);
    address.sin_port = htons(server_port);

    // Bind the server socket to the IP and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        log_message("Failed to bind server socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }   

    // Start listening for incoming connections
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        log_message("Failed to listen on server socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    char log[256];
    snprintf(log, sizeof(log), "Server listening on %s:%d", server_ip, server_port);
    log_message(log);
    printf("Server listening on %s:%d...\n", server_ip, server_port);
}

// Function to handle a client request
void *handle_client(void *arg) {
    int socket_id = *(int *)arg;
    free(arg);

    // Get the client IP address
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    // Get the client address information
    getpeername(socket_id, (struct sockaddr *)&client_addr, &client_addr_len);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

    char buffer[BUFSIZE];
    char method[16];
    char path[256];

    // Read the client request
    memset(buffer, 0, BUFSIZE);
    ssize_t bytes_read = read(socket_id, buffer, BUFSIZE - 1);
    if (bytes_read <= 0) {
        perror("read");
        log_message("Failed to read client request");
        close(socket_id);
        return NULL;
    }
    buffer[bytes_read] = '\0';
    printf("Request received:\n%s\n", buffer);

    parse_request(buffer, method, sizeof(method), path, sizeof(path));
    printf("Method: %s, Path: %s\n", method, path);

    // Handle the client request based on the HTTP method
    if (strcmp(method, "GET") == 0) {
        handle_request(socket_id, client_ip, method, path);
    } else {
        log_message("Unsupported HTTP method");
        send_response(socket_id, HTTP_405, NULL);
        log_access(client_ip, method, path, 405);
    }
    close(socket_id);
    return NULL;
}
