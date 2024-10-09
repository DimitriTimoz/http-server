#include "server.h"

// Function to load the server configuration from the configuration file
void load_config() {
    FILE *config_file = fopen(CONFIG_FILE_1, "r");
    if (config_file == NULL) {
        config_file = fopen(CONFIG_FILE_2, "r");
    }
    if (config_file) {
        char line[256];
        while (fgets(line, sizeof(line), config_file)) {
            char key[256], value[256];
            if (sscanf(line, " %255[^= ] = %255s", key, value) == 2) {
                if (strcasecmp(key, "server_ip") == 0) {
                    strncpy(server_ip, value, sizeof(server_ip) - 1);
                } else if (strcasecmp(key, "server_port") == 0) {
                    server_port = atoi(value);
                } else if (strcasecmp(key, "server_root") == 0) {
                    strncpy(web_root, value, sizeof(web_root) - 1);
                } else if (strcasecmp(key, "log_file") == 0) {
                    strncpy(log_file_path, value, sizeof(log_file_path) - 1);
                }
            }
        }
        fclose(config_file);
    }
    char log[256];
    int ret = snprintf(log, sizeof(log), "Configuration loaded: IP=%s, Port=%d, Root=%s", server_ip, server_port, web_root);
    if (ret >= sizeof(log)) {
        // La chaîne a été tronquée
        fprintf(stderr, "Log message truncated\n");
    }
    log_message(log);
}
