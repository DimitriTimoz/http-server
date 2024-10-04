#include "server.h"

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
