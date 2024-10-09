#include "server.h"


// Function to create the log file
void create_log_file(char* file_path) {
    FILE *log_file = fopen(file_path, "a");
    if (log_file) {
        fclose(log_file);
    } else {
        perror("Failed to create log file");
    }
}

void log_message(const char *message) {
    FILE *log_file = fopen(log_file_path, "a");
    if (log_file) {
        time_t now = time(NULL);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(log_file, "%s: %s\n", timestamp, message);
        printf("%s: %s\n", timestamp, message);
        fclose(log_file);
    } else {
        create_log_file(log_file_path);
        perror("Failed to open log file");
    }
}

// Function to log access information
void log_access(const char *client_ip, const char *method, const char *path, int status_code) {
    FILE *access_log_file = fopen(ACCESS_LOG_FILE, "a");
    if (access_log_file) {
        time_t now = time(NULL);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(access_log_file, "%s - %s - %s %s - %d\n", timestamp, client_ip, method, path, status_code);
        fclose(access_log_file);
    } else {
        create_log_file(ACCESS_LOG_FILE);
        perror("Failed to open access log file");
    }
}
