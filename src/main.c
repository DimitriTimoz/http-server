#include "server.h"

int main() {
    load_config();

    int server_fd = create_server_socket();
    bind_and_listen(server_fd);

    struct sockaddr_in address;
    int addrlen = sizeof(address);

    while (1) {
        int *socket_id = malloc(sizeof(int));
        if (socket_id == NULL) {
            log_message("Failed to allocate memory for client socket");
            continue;
        }

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
