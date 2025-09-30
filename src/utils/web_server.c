/*
 * ========================================
 * Web Server Implementation - HTTP Server ve REST API
 * ========================================
 */

#include "../../include/web_server.h"
#include "../../include/logger.h"
#include "../../lib/cJSON/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Global variables
static int server_socket = -1;
static bool server_running = false;
static WebServerConfig config;

// Initialize web server
bool init_web_server(int port) {
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        log_error("WSAStartup failed");
        return false;
    }
    #endif

    // Initialize configuration
    config.port = port > 0 ? port : DEFAULT_PORT;
    config.max_connections = MAX_CONNECTIONS;
    config.enable_cors = true;
    strcpy(config.static_path, "web");
    strcpy(config.log_file, "logs/web_server.log");

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        log_error("Failed to create socket");
        return false;
    }

    // Set socket options
    int opt = 1;
    #ifdef _WIN32
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
    #else
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    #endif
        log_error("Failed to set socket options");
        return false;
    }

    // Bind socket
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(config.port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        log_error("Failed to bind socket to port %d", config.port);
        return false;
    }

    log_info("Web server initialized on port %d", config.port);
    return true;
}

// Start web server
bool start_web_server(void) {
    if (server_socket < 0) {
        log_error("Web server not initialized");
        return false;
    }

    // Listen for connections
    if (listen(server_socket, config.max_connections) < 0) {
        log_error("Failed to listen on socket");
        return false;
    }

    server_running = true;
    log_info("Web server started on http://localhost:%d", config.port);

    // Main server loop
    while (server_running) {
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            if (server_running) {
                log_error("Failed to accept client connection");
            }
            continue;
        }

        // Handle client request
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));
        
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            
            HttpRequest request;
            HttpResponse response;
            
            if (parse_http_request(buffer, &request)) {
                // Route request
                if (strncmp(request.path, "/api/", 5) == 0) {
                    handle_api_request(&request, &response);
                } else if (strcmp(request.path, "/") == 0 || strcmp(request.path, "/dashboard") == 0) {
                    serve_dashboard(&response);
                } else {
                    serve_static_file(request.path, &response);
                }
            } else {
                create_http_response(&response, HTTP_400_BAD_REQUEST, "text/plain", "Bad Request");
            }

            // Add CORS headers if enabled
            if (config.enable_cors) {
                add_cors_headers(&response);
            }

            send_http_response(client_socket, &response);
        }

        #ifdef _WIN32
        closesocket(client_socket);
        #else
        close(client_socket);
        #endif
    }

    return true;
}

// Parse HTTP request
bool parse_http_request(const char* raw_request, HttpRequest* request) {
    if (!raw_request || !request) {
        return false;
    }

    // Parse request line
    char method_str[16];
    char path_with_query[1024];
    
    if (sscanf(raw_request, "%15s %1023s", method_str, path_with_query) != 2) {
        return false;
    }

    // Parse method
    if (strcmp(method_str, "GET") == 0) {
        request->method = HTTP_GET;
    } else if (strcmp(method_str, "POST") == 0) {
        request->method = HTTP_POST;
    } else if (strcmp(method_str, "PUT") == 0) {
        request->method = HTTP_PUT;
    } else if (strcmp(method_str, "DELETE") == 0) {
        request->method = HTTP_DELETE;
    } else if (strcmp(method_str, "OPTIONS") == 0) {
        request->method = HTTP_OPTIONS;
    } else {
        return false;
    }

    // Parse path and query string
    char* query_start = strchr(path_with_query, '?');
    if (query_start) {
        *query_start = '\0';
        strncpy(request->query_string, query_start + 1, sizeof(request->query_string) - 1);
        request->query_string[sizeof(request->query_string) - 1] = '\0';
    } else {
        request->query_string[0] = '\0';
    }
    
    strncpy(request->path, path_with_query, sizeof(request->path) - 1);
    request->path[sizeof(request->path) - 1] = '\0';

    // Parse headers and body (simplified)
    const char* body_start = strstr(raw_request, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
        strncpy(request->body, body_start, sizeof(request->body) - 1);
        request->body[sizeof(request->body) - 1] = '\0';
        request->content_length = strlen(request->body);
    } else {
        request->body[0] = '\0';
        request->content_length = 0;
    }

    return true;
}

// Create HTTP response
void create_http_response(HttpResponse* response, HttpStatus status, const char* content_type, const char* body) {
    response->status = status;
    strncpy(response->content_type, content_type, sizeof(response->content_type) - 1);
    response->content_type[sizeof(response->content_type) - 1] = '\0';
    
    if (body) {
        strncpy(response->body, body, sizeof(response->body) - 1);
        response->body[sizeof(response->body) - 1] = '\0';
        response->content_length = strlen(response->body);
    } else {
        response->body[0] = '\0';
        response->content_length = 0;
    }
    
    response->headers[0] = '\0';
}

// Send HTTP response
void send_http_response(int client_socket, const HttpResponse* response) {
    char response_buffer[MAX_RESPONSE_SIZE];
    
    // Status line
    const char* status_text;
    switch (response->status) {
        case HTTP_200_OK: status_text = "OK"; break;
        case HTTP_201_CREATED: status_text = "Created"; break;
        case HTTP_400_BAD_REQUEST: status_text = "Bad Request"; break;
        case HTTP_401_UNAUTHORIZED: status_text = "Unauthorized"; break;
        case HTTP_404_NOT_FOUND: status_text = "Not Found"; break;
        case HTTP_405_METHOD_NOT_ALLOWED: status_text = "Method Not Allowed"; break;
        case HTTP_500_INTERNAL_ERROR: status_text = "Internal Server Error"; break;
        default: status_text = "Unknown"; break;
    }

    snprintf(response_buffer, sizeof(response_buffer),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Server: AutomationCenter/1.0\r\n"
        "%s"
        "\r\n"
        "%s",
        response->status, status_text,
        response->content_type,
        response->content_length,
        response->headers,
        response->body
    );

    send(client_socket, response_buffer, strlen(response_buffer), 0);
}

// Handle API requests
void handle_api_request(const HttpRequest* request, HttpResponse* response) {
    if (strncmp(request->path, "/api/system-metrics", 19) == 0) {
        api_get_system_metrics(request, response);
    } else if (strncmp(request->path, "/api/file-operations", 20) == 0) {
        api_get_file_operations(request, response);
    } else if (strncmp(request->path, "/api/network-metrics", 20) == 0) {
        api_get_network_metrics(request, response);
    } else if (strncmp(request->path, "/api/security-scans", 19) == 0) {
        api_get_security_scans(request, response);
    } else if (strncmp(request->path, "/api/scheduled-tasks", 20) == 0) {
        api_get_scheduled_tasks(request, response);
    } else {
        create_http_response(response, HTTP_404_NOT_FOUND, "application/json", 
            "{\"error\":\"API endpoint not found\"}");
    }
}

// API: Get system metrics
void api_get_system_metrics(const HttpRequest* request, HttpResponse* response) {
    SystemMetrics* metrics = NULL;
    int count = 0;
    int limit = 50; // Default limit

    // Parse limit from query string
    if (strlen(request->query_string) > 0) {
        char* limit_param = strstr(request->query_string, "limit=");
        if (limit_param) {
            limit = atoi(limit_param + 6);
            if (limit <= 0 || limit > 1000) limit = 50;
        }
    }

    if (get_system_metrics_history(&metrics, &count, limit)) {
        cJSON* json = cJSON_CreateObject();
        cJSON* data_array = cJSON_CreateArray();

        for (int i = 0; i < count; i++) {
            cJSON* item = cJSON_CreateObject();
            cJSON_AddNumberToObject(item, "id", metrics[i].id);
            cJSON_AddNumberToObject(item, "timestamp", metrics[i].timestamp);
            cJSON_AddNumberToObject(item, "cpu_usage", metrics[i].cpu_usage);
            cJSON_AddNumberToObject(item, "memory_usage", metrics[i].memory_usage);
            cJSON_AddNumberToObject(item, "disk_usage", metrics[i].disk_usage);
            cJSON_AddStringToObject(item, "hostname", metrics[i].hostname);
            cJSON_AddItemToArray(data_array, item);
        }

        cJSON_AddItemToObject(json, "data", data_array);
        cJSON_AddNumberToObject(json, "count", count);

        char* json_string = cJSON_Print(json);
        create_http_response(response, HTTP_200_OK, "application/json", json_string);

        free(json_string);
        cJSON_Delete(json);
        if (metrics) free(metrics);
    } else {
        create_http_response(response, HTTP_500_INTERNAL_ERROR, "application/json", 
            "{\"error\":\"Failed to retrieve system metrics\"}");
    }
}

// API: Get file operations
void api_get_file_operations(const HttpRequest* request, HttpResponse* response) {
    FileOperation* operations = NULL;
    int count = 0;
    int limit = 50;

    if (strlen(request->query_string) > 0) {
        char* limit_param = strstr(request->query_string, "limit=");
        if (limit_param) {
            limit = atoi(limit_param + 6);
            if (limit <= 0 || limit > 1000) limit = 50;
        }
    }

    if (get_file_operations_history(&operations, &count, limit)) {
        cJSON* json = cJSON_CreateObject();
        cJSON* data_array = cJSON_CreateArray();

        for (int i = 0; i < count; i++) {
            cJSON* item = cJSON_CreateObject();
            cJSON_AddNumberToObject(item, "id", operations[i].id);
            cJSON_AddNumberToObject(item, "timestamp", operations[i].timestamp);
            cJSON_AddStringToObject(item, "operation", operations[i].operation);
            cJSON_AddStringToObject(item, "file_path", operations[i].file_path);
            cJSON_AddNumberToObject(item, "file_size", operations[i].file_size);
            cJSON_AddStringToObject(item, "status", operations[i].status);
            cJSON_AddItemToArray(data_array, item);
        }

        cJSON_AddItemToObject(json, "data", data_array);
        cJSON_AddNumberToObject(json, "count", count);

        char* json_string = cJSON_Print(json);
        create_http_response(response, HTTP_200_OK, "application/json", json_string);

        free(json_string);
        cJSON_Delete(json);
        if (operations) free(operations);
    } else {
        create_http_response(response, HTTP_500_INTERNAL_ERROR, "application/json", 
            "{\"error\":\"Failed to retrieve file operations\"}");
    }
}

// API: Get network metrics
void api_get_network_metrics(const HttpRequest* request, HttpResponse* response) {
    NetworkMetrics* metrics = NULL;
    int count = 0;
    int limit = 50;

    if (strlen(request->query_string) > 0) {
        char* limit_param = strstr(request->query_string, "limit=");
        if (limit_param) {
            limit = atoi(limit_param + 6);
            if (limit <= 0 || limit > 1000) limit = 50;
        }
    }

    if (get_network_metrics_history(&metrics, &count, limit)) {
        cJSON* json = cJSON_CreateObject();
        cJSON* data_array = cJSON_CreateArray();

        for (int i = 0; i < count; i++) {
            cJSON* item = cJSON_CreateObject();
            cJSON_AddNumberToObject(item, "id", metrics[i].id);
            cJSON_AddNumberToObject(item, "timestamp", metrics[i].timestamp);
            cJSON_AddStringToObject(item, "target", metrics[i].target);
            cJSON_AddNumberToObject(item, "ping_time", metrics[i].ping_time);
            cJSON_AddBoolToObject(item, "connection_status", metrics[i].connection_status);
            cJSON_AddStringToObject(item, "interface_name", metrics[i].interface_name);
            cJSON_AddItemToArray(data_array, item);
        }

        cJSON_AddItemToObject(json, "data", data_array);
        cJSON_AddNumberToObject(json, "count", count);

        char* json_string = cJSON_Print(json);
        create_http_response(response, HTTP_200_OK, "application/json", json_string);

        free(json_string);
        cJSON_Delete(json);
        if (metrics) free(metrics);
    } else {
        create_http_response(response, HTTP_500_INTERNAL_ERROR, "application/json", 
            "{\"error\":\"Failed to retrieve network metrics\"}");
    }
}

// API: Get security scans
void api_get_security_scans(const HttpRequest* request, HttpResponse* response) {
    SecurityScan* scans = NULL;
    int count = 0;
    int limit = 50;

    if (strlen(request->query_string) > 0) {
        char* limit_param = strstr(request->query_string, "limit=");
        if (limit_param) {
            limit = atoi(limit_param + 6);
            if (limit <= 0 || limit > 1000) limit = 50;
        }
    }

    if (get_security_scans_history(&scans, &count, limit)) {
        cJSON* json = cJSON_CreateObject();
        cJSON* data_array = cJSON_CreateArray();

        for (int i = 0; i < count; i++) {
            cJSON* item = cJSON_CreateObject();
            cJSON_AddNumberToObject(item, "id", scans[i].id);
            cJSON_AddNumberToObject(item, "timestamp", scans[i].timestamp);
            cJSON_AddStringToObject(item, "scan_type", scans[i].scan_type);
            cJSON_AddStringToObject(item, "target", scans[i].target);
            cJSON_AddNumberToObject(item, "threats_found", scans[i].threats_found);
            cJSON_AddStringToObject(item, "severity", scans[i].severity);
            cJSON_AddStringToObject(item, "description", scans[i].description);
            cJSON_AddItemToArray(data_array, item);
        }

        cJSON_AddItemToObject(json, "data", data_array);
        cJSON_AddNumberToObject(json, "count", count);

        char* json_string = cJSON_Print(json);
        create_http_response(response, HTTP_200_OK, "application/json", json_string);

        free(json_string);
        cJSON_Delete(json);
        if (scans) free(scans);
    } else {
        create_http_response(response, HTTP_500_INTERNAL_ERROR, "application/json", 
            "{\"error\":\"Failed to retrieve security scans\"}");
    }
}

// API: Get scheduled tasks
void api_get_scheduled_tasks(const HttpRequest* request, HttpResponse* response) {
    ScheduledTask* tasks = NULL;
    int count = 0;

    if (get_scheduled_tasks(&tasks, &count)) {
        cJSON* json = cJSON_CreateObject();
        cJSON* data_array = cJSON_CreateArray();

        for (int i = 0; i < count; i++) {
            cJSON* item = cJSON_CreateObject();
            cJSON_AddNumberToObject(item, "id", tasks[i].id);
            cJSON_AddStringToObject(item, "task_name", tasks[i].task_name);
            cJSON_AddStringToObject(item, "command", tasks[i].command);
            cJSON_AddStringToObject(item, "schedule", tasks[i].schedule);
            cJSON_AddNumberToObject(item, "next_run", tasks[i].next_run);
            cJSON_AddNumberToObject(item, "last_run", tasks[i].last_run);
            cJSON_AddBoolToObject(item, "enabled", tasks[i].enabled);
            cJSON_AddStringToObject(item, "status", tasks[i].status);
            cJSON_AddItemToArray(data_array, item);
        }

        cJSON_AddItemToObject(json, "data", data_array);
        cJSON_AddNumberToObject(json, "count", count);

        char* json_string = cJSON_Print(json);
        create_http_response(response, HTTP_200_OK, "application/json", json_string);

        free(json_string);
        cJSON_Delete(json);
        if (tasks) free(tasks);
    } else {
        create_http_response(response, HTTP_500_INTERNAL_ERROR, "application/json", 
            "{\"error\":\"Failed to retrieve scheduled tasks\"}");
    }
}

// Serve dashboard HTML
void serve_dashboard(HttpResponse* response) {
    const char* dashboard_html = 
        "<!DOCTYPE html>\n"
        "<html lang=\"tr\">\n"
        "<head>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "    <title>Sistem Otomasyon Merkezi - Dashboard</title>\n"
        "    <style>\n"
        "        * { margin: 0; padding: 0; box-sizing: border-box; }\n"
        "        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #f5f5f5; }\n"
        "        .header { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 1rem; text-align: center; }\n"
        "        .container { max-width: 1200px; margin: 2rem auto; padding: 0 1rem; }\n"
        "        .dashboard-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 1.5rem; }\n"
        "        .card { background: white; border-radius: 8px; padding: 1.5rem; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }\n"
        "        .card h3 { color: #333; margin-bottom: 1rem; border-bottom: 2px solid #667eea; padding-bottom: 0.5rem; }\n"
        "        .metric { display: flex; justify-content: space-between; margin: 0.5rem 0; }\n"
        "        .metric-value { font-weight: bold; color: #667eea; }\n"
        "        .status-indicator { width: 12px; height: 12px; border-radius: 50%; display: inline-block; margin-right: 8px; }\n"
        "        .status-online { background: #4CAF50; }\n"
        "        .status-offline { background: #f44336; }\n"
        "        .refresh-btn { background: #667eea; color: white; border: none; padding: 0.5rem 1rem; border-radius: 4px; cursor: pointer; }\n"
        "        .refresh-btn:hover { background: #5a6fd8; }\n"
        "        .loading { text-align: center; color: #666; }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <div class=\"header\">\n"
        "        <h1>🖥️ Sistem Otomasyon Merkezi</h1>\n"
        "        <p>Gerçek Zamanlı Sistem İzleme Dashboard'u</p>\n"
        "    </div>\n"
        "    <div class=\"container\">\n"
        "        <div class=\"dashboard-grid\">\n"
        "            <div class=\"card\">\n"
        "                <h3>📊 Sistem Metrikleri</h3>\n"
        "                <button class=\"refresh-btn\" onclick=\"loadSystemMetrics()\">Yenile</button>\n"
        "                <div id=\"system-metrics\" class=\"loading\">Yükleniyor...</div>\n"
        "            </div>\n"
        "            <div class=\"card\">\n"
        "                <h3>📁 Dosya İşlemleri</h3>\n"
        "                <button class=\"refresh-btn\" onclick=\"loadFileOperations()\">Yenile</button>\n"
        "                <div id=\"file-operations\" class=\"loading\">Yükleniyor...</div>\n"
        "            </div>\n"
        "            <div class=\"card\">\n"
        "                <h3>🌐 Ağ Metrikleri</h3>\n"
        "                <button class=\"refresh-btn\" onclick=\"loadNetworkMetrics()\">Yenile</button>\n"
        "                <div id=\"network-metrics\" class=\"loading\">Yükleniyor...</div>\n"
        "            </div>\n"
        "            <div class=\"card\">\n"
        "                <h3>🔒 Güvenlik Taramaları</h3>\n"
        "                <button class=\"refresh-btn\" onclick=\"loadSecurityScans()\">Yenile</button>\n"
        "                <div id=\"security-scans\" class=\"loading\">Yükleniyor...</div>\n"
        "            </div>\n"
        "            <div class=\"card\">\n"
        "                <h3>⏰ Zamanlanmış Görevler</h3>\n"
        "                <button class=\"refresh-btn\" onclick=\"loadScheduledTasks()\">Yenile</button>\n"
        "                <div id=\"scheduled-tasks\" class=\"loading\">Yükleniyor...</div>\n"
        "            </div>\n"
        "        </div>\n"
        "    </div>\n"
        "    <script>\n"
        "        async function fetchAPI(endpoint) {\n"
        "            try {\n"
        "                const response = await fetch(`/api/${endpoint}?limit=5`);\n"
        "                return await response.json();\n"
        "            } catch (error) {\n"
        "                console.error('API Error:', error);\n"
        "                return { error: 'Veri yüklenemedi' };\n"
        "            }\n"
        "        }\n"
        "        async function loadSystemMetrics() {\n"
        "            const container = document.getElementById('system-metrics');\n"
        "            container.innerHTML = 'Yükleniyor...';\n"
        "            const data = await fetchAPI('system-metrics');\n"
        "            if (data.error) {\n"
        "                container.innerHTML = `<p style=\"color: red;\">${data.error}</p>`;\n"
        "                return;\n"
        "            }\n"
        "            let html = '';\n"
        "            if (data.data && data.data.length > 0) {\n"
        "                const latest = data.data[0];\n"
        "                html = `\n"
        "                    <div class=\"metric\"><span>CPU Kullanımı:</span><span class=\"metric-value\">${latest.cpu_usage.toFixed(1)}%</span></div>\n"
        "                    <div class=\"metric\"><span>Bellek Kullanımı:</span><span class=\"metric-value\">${latest.memory_usage.toFixed(1)}%</span></div>\n"
        "                    <div class=\"metric\"><span>Disk Kullanımı:</span><span class=\"metric-value\">${latest.disk_usage.toFixed(1)}%</span></div>\n"
        "                    <div class=\"metric\"><span>Hostname:</span><span class=\"metric-value\">${latest.hostname}</span></div>\n"
        "                    <div class=\"metric\"><span>Son Güncelleme:</span><span class=\"metric-value\">${new Date(latest.timestamp * 1000).toLocaleString('tr-TR')}</span></div>\n"
        "                `;\n"
        "            } else {\n"
        "                html = '<p>Henüz veri bulunmuyor</p>';\n"
        "            }\n"
        "            container.innerHTML = html;\n"
        "        }\n"
        "        async function loadFileOperations() {\n"
        "            const container = document.getElementById('file-operations');\n"
        "            container.innerHTML = 'Yükleniyor...';\n"
        "            const data = await fetchAPI('file-operations');\n"
        "            if (data.error) {\n"
        "                container.innerHTML = `<p style=\"color: red;\">${data.error}</p>`;\n"
        "                return;\n"
        "            }\n"
        "            let html = '';\n"
        "            if (data.data && data.data.length > 0) {\n"
        "                data.data.slice(0, 3).forEach(op => {\n"
        "                    html += `<div class=\"metric\"><span>${op.operation}:</span><span class=\"metric-value\">${op.file_path.split('/').pop()}</span></div>`;\n"
        "                });\n"
        "                html += `<p><small>Toplam ${data.count} işlem</small></p>`;\n"
        "            } else {\n"
        "                html = '<p>Henüz dosya işlemi bulunmuyor</p>';\n"
        "            }\n"
        "            container.innerHTML = html;\n"
        "        }\n"
        "        async function loadNetworkMetrics() {\n"
        "            const container = document.getElementById('network-metrics');\n"
        "            container.innerHTML = 'Yükleniyor...';\n"
        "            const data = await fetchAPI('network-metrics');\n"
        "            if (data.error) {\n"
        "                container.innerHTML = `<p style=\"color: red;\">${data.error}</p>`;\n"
        "                return;\n"
        "            }\n"
        "            let html = '';\n"
        "            if (data.data && data.data.length > 0) {\n"
        "                data.data.slice(0, 3).forEach(metric => {\n"
        "                    const statusClass = metric.connection_status ? 'status-online' : 'status-offline';\n"
        "                    html += `<div class=\"metric\"><span><span class=\"status-indicator ${statusClass}\"></span>${metric.target}:</span><span class=\"metric-value\">${metric.ping_time}ms</span></div>`;\n"
        "                });\n"
        "                html += `<p><small>Toplam ${data.count} ölçüm</small></p>`;\n"
        "            } else {\n"
        "                html = '<p>Henüz ağ ölçümü bulunmuyor</p>';\n"
        "            }\n"
        "            container.innerHTML = html;\n"
        "        }\n"
        "        async function loadSecurityScans() {\n"
        "            const container = document.getElementById('security-scans');\n"
        "            container.innerHTML = 'Yükleniyor...';\n"
        "            const data = await fetchAPI('security-scans');\n"
        "            if (data.error) {\n"
        "                container.innerHTML = `<p style=\"color: red;\">${data.error}</p>`;\n"
        "                return;\n"
        "            }\n"
        "            let html = '';\n"
        "            if (data.data && data.data.length > 0) {\n"
        "                data.data.slice(0, 3).forEach(scan => {\n"
        "                    const severityColor = scan.severity === 'high' ? 'red' : scan.severity === 'medium' ? 'orange' : 'green';\n"
        "                    html += `<div class=\"metric\"><span>${scan.scan_type}:</span><span class=\"metric-value\" style=\"color: ${severityColor}\">${scan.threats_found} tehdit</span></div>`;\n"
        "                });\n"
        "                html += `<p><small>Toplam ${data.count} tarama</small></p>`;\n"
        "            } else {\n"
        "                html = '<p>Henüz güvenlik taraması bulunmuyor</p>';\n"
        "            }\n"
        "            container.innerHTML = html;\n"
        "        }\n"
        "        async function loadScheduledTasks() {\n"
        "            const container = document.getElementById('scheduled-tasks');\n"
        "            container.innerHTML = 'Yükleniyor...';\n"
        "            const data = await fetchAPI('scheduled-tasks');\n"
        "            if (data.error) {\n"
        "                container.innerHTML = `<p style=\"color: red;\">${data.error}</p>`;\n"
        "                return;\n"
        "            }\n"
        "            let html = '';\n"
        "            if (data.data && data.data.length > 0) {\n"
        "                data.data.slice(0, 3).forEach(task => {\n"
        "                    const statusClass = task.enabled ? 'status-online' : 'status-offline';\n"
        "                    html += `<div class=\"metric\"><span><span class=\"status-indicator ${statusClass}\"></span>${task.task_name}:</span><span class=\"metric-value\">${task.status}</span></div>`;\n"
        "                });\n"
        "                html += `<p><small>Toplam ${data.count} görev</small></p>`;\n"
        "            } else {\n"
        "                html = '<p>Henüz zamanlanmış görev bulunmuyor</p>';\n"
        "            }\n"
        "            container.innerHTML = html;\n"
        "        }\n"
        "        // Load all data on page load\n"
        "        window.onload = function() {\n"
        "            loadSystemMetrics();\n"
        "            loadFileOperations();\n"
        "            loadNetworkMetrics();\n"
        "            loadSecurityScans();\n"
        "            loadScheduledTasks();\n"
        "            // Auto-refresh every 30 seconds\n"
        "            setInterval(() => {\n"
        "                loadSystemMetrics();\n"
        "                loadNetworkMetrics();\n"
        "            }, 30000);\n"
        "        };\n"
        "    </script>\n"
        "</body>\n"
        "</html>";

    create_http_response(response, HTTP_200_OK, "text/html", dashboard_html);
}

// Add CORS headers
void add_cors_headers(HttpResponse* response) {
    strcat(response->headers, "Access-Control-Allow-Origin: *\r\n");
    strcat(response->headers, "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n");
    strcat(response->headers, "Access-Control-Allow-Headers: Content-Type, Authorization\r\n");
}

// Serve static files (placeholder)
void serve_static_file(const char* file_path, HttpResponse* response) {
    create_http_response(response, HTTP_404_NOT_FOUND, "text/plain", "File not found");
}

// Get MIME type (placeholder)
const char* get_mime_type(const char* file_extension) {
    if (strcmp(file_extension, ".html") == 0) return "text/html";
    if (strcmp(file_extension, ".css") == 0) return "text/css";
    if (strcmp(file_extension, ".js") == 0) return "application/javascript";
    if (strcmp(file_extension, ".json") == 0) return "application/json";
    return "text/plain";
}

// Cleanup web server
void cleanup_web_server(void) {
    server_running = false;
    
    if (server_socket >= 0) {
        #ifdef _WIN32
        closesocket(server_socket);
        WSACleanup();
        #else
        close(server_socket);
        #endif
        server_socket = -1;
    }
    
    log_info("Web server stopped");
}

// Stop web server
void stop_web_server(void) {
    server_running = false;
}