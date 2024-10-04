#include "server.h"

void parse_request(const char *buffer, char *method, size_t method_size, char *path, size_t path_size) {
    sscanf(buffer, "%15s %255s", method, path);
    if (strcmp(path, "/") == 0) {
        strncpy(path, "/index.html", path_size - 1);
        path[path_size - 1] = '\0';
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
        if (strcmp(ext, ".json") == 0) return "application/json";
        if (strcmp(ext, ".xml") == 0) return "application/xml";
        if (strcmp(ext, ".pdf") == 0) return "application/pdf";
        if (strcmp(ext, ".zip") == 0) return "application/zip";
        if (strcmp(ext, ".tar") == 0) return "application/x-tar";
        if (strcmp(ext, ".gz") == 0) return "application/gzip";
        if (strcmp(ext, ".mp3") == 0) return "audio/mpeg";
        if (strcmp(ext, ".mp4") == 0) return "video/mp4";
        if (strcmp(ext, ".avi") == 0) return "video/x-msvideo";
        if (strcmp(ext, ".mpeg") == 0) return "video/mpeg";
        if (strcmp(ext, ".woff") == 0) return "font/woff";
        if (strcmp(ext, ".woff2") == 0) return "font/woff2";
        if (strcmp(ext, ".ttf") == 0) return "font/ttf";
        if (strcmp(ext, ".otf") == 0) return "font/otf";
        if (strcmp(ext, ".csv") == 0) return "text/csv";
        if (strcmp(ext, ".svg") == 0) return "image/svg+xml";
        if (strcmp(ext, ".ico") == 0) return "image/x-icon";
        if (strcmp(ext, ".bmp") == 0) return "image/bmp";
        if (strcmp(ext, ".wav") == 0) return "audio/wav";
        if (strcmp(ext, ".webm") == 0) return "video/webm";
        if (strcmp(ext, ".webp") == 0) return "image/webp";
    }
    return "application/octet-stream";
}

int is_safe_path(const char *base_path, const char *requested_path) {
    char real_base[512];
    char real_requested[512];

    if (realpath(base_path, real_base) == NULL || realpath(requested_path, real_requested) == NULL) {
        return 0;
    }

    return strstr(real_requested, real_base) == real_requested;
}
