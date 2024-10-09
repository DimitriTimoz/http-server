#include "server.h"

// Function to decode URL-encoded strings
char *url_decode(const char *src, char *dest, size_t dest_size) {
    char *p = dest;
    const char *end = dest + dest_size - 1;
    while (*src && p < end) {
        if (*src == '%') {
            if (isxdigit((unsigned char)src[1]) && isxdigit((unsigned char)src[2])) {
                char hex[3] = { src[1], src[2], '\0' };
                *p++ = (char)strtol(hex, NULL, 16);
                src += 3;
            } else {
                *p++ = '%';
                src++;
            }
        } else if (*src == '+') {
            *p++ = ' ';
            src++;
        } else {
            *p++ = *src++;
        }
    }
    *p = '\0';
    return dest;
}

// Function to parse the HTTP request line and extract the method and path
void parse_request(const char *buffer, char *method, size_t method_size, char *path, size_t path_size) {
    method[0] = '\0';
    path[0] = '\0';
    sscanf(buffer, "%15s %255s", method, path);
    if (strcmp(path, "/") == 0) {
        snprintf(path, path_size, "/index.html");
    } else {
        char decoded_path[256];
        url_decode(path, decoded_path, sizeof(decoded_path));
        snprintf(path, path_size, "%s", decoded_path);
    }
}

// Function to determine the content type based on the file extension
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

// Function to check if the requested path is within the server's root directory
int is_safe_path(const char *base_path, const char *requested_path) {
    char real_base[512];
    char real_requested[512];

    if (realpath(base_path, real_base) == NULL || realpath(requested_path, real_requested) == NULL) {
        return 0;
    }

    return strstr(real_requested, real_base) == real_requested;
}
