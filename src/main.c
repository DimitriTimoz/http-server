#include "server.h"

int main() {
    // Load the server configuration from the configuration file
    load_config();

    // Create a server socket
    int server_fd = create_server_socket();
    // Bind the server socket to the IP and port and start listening for connections
    bind_and_listen(server_fd);

    // Structure to hold client address information
    struct sockaddr_in address;
    // Length of the address structure
    int addrlen = sizeof(address);

    while (1) {
        // Allocate memory for the client socket descriptor
        int *socket_id = malloc(sizeof(int));
        // Check if memory allocation was successful
        if (socket_id == NULL) {
            log_message("Failed to allocate memory for client socket");
            continue;
        }

        // Accept an incoming client connection
        *socket_id = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (*socket_id < 0) {
            perror("accept");
            log_message("Failed to accept client connection");
            free(socket_id);
            continue;
        }

        // Create a new thread to handle the client request
        pthread_t thread_id;
        // Start the client handler thread
        if (pthread_create(&thread_id, NULL, handle_client, socket_id) != 0) {
            perror("pthread_create");
            log_message("Failed to create thread for client");
            free(socket_id);
        } else {
            // Detach the thread so that resources are released when it finishes
            pthread_detach(thread_id);
        }
    }

    // Close the server socket before exiting
    close(server_fd);
    return 0;
}
