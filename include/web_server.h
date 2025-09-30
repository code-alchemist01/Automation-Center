/*
 * ========================================
 * Web Server Header - HTTP Server ve REST API
 * ========================================
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "core.h"
#include "database.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

// Web server configuration
#define DEFAULT_PORT 8080
#define MAX_CONNECTIONS 10
#define BUFFER_SIZE 8192
#define MAX_RESPONSE_SIZE 65536

// HTTP methods
typedef enum {
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE,
    HTTP_OPTIONS
} HttpMethod;

// HTTP status codes
typedef enum {
    HTTP_200_OK = 200,
    HTTP_201_CREATED = 201,
    HTTP_400_BAD_REQUEST = 400,
    HTTP_401_UNAUTHORIZED = 401,
    HTTP_404_NOT_FOUND = 404,
    HTTP_405_METHOD_NOT_ALLOWED = 405,
    HTTP_500_INTERNAL_ERROR = 500
} HttpStatus;

// HTTP request structure
typedef struct {
    HttpMethod method;
    char path[512];
    char query_string[1024];
    char headers[2048];
    char body[4096];
    int content_length;
} HttpRequest;

// HTTP response structure
typedef struct {
    HttpStatus status;
    char content_type[128];
    char headers[1024];
    char body[MAX_RESPONSE_SIZE];
    int content_length;
} HttpResponse;

// Web server configuration
typedef struct {
    int port;
    int max_connections;
    bool enable_cors;
    char static_path[MAX_PATH_LEN];
    char log_file[MAX_PATH_LEN];
} WebServerConfig;

// Function prototypes
bool init_web_server(int port);
void cleanup_web_server(void);
bool start_web_server(void);
void stop_web_server(void);

// HTTP handling functions
bool parse_http_request(const char* raw_request, HttpRequest* request);
void create_http_response(HttpResponse* response, HttpStatus status, const char* content_type, const char* body);
void send_http_response(int client_socket, const HttpResponse* response);

// REST API endpoints
void handle_api_request(const HttpRequest* request, HttpResponse* response);
void api_get_system_metrics(const HttpRequest* request, HttpResponse* response);
void api_get_file_operations(const HttpRequest* request, HttpResponse* response);
void api_get_network_metrics(const HttpRequest* request, HttpResponse* response);
void api_get_security_scans(const HttpRequest* request, HttpResponse* response);
void api_get_scheduled_tasks(const HttpRequest* request, HttpResponse* response);

// Static file serving
void serve_static_file(const char* file_path, HttpResponse* response);
void serve_dashboard(HttpResponse* response);

// Utility functions
const char* get_mime_type(const char* file_extension);
char* url_decode(const char* encoded);
char* json_escape_string(const char* str);

// CORS handling
void add_cors_headers(HttpResponse* response);

#endif // WEB_SERVER_H