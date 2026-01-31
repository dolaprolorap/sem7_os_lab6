#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "repository.h"
#include "server.h"

#define PORT 8080
#define BUFFER_SIZE 1024

void get_start_finish(char *query, time_t *start, time_t *finish)
{
    query = strchr(query, '?');

    do
    {
        query++;

        if (strncmp(query, "start", 5) == 0)
        {
            query = strchr(query, '=');
            query++;

            *start = strtol(query, NULL, 10);
        }

        if (strncmp(query, "finish", 6) == 0)
        {
            query = strchr(query, '=');
            query++;

            *finish = strtol(query, NULL, 10);
        }
    } while (query = strchr(query, '&'));
}

void parse_to_json(double *data, size_t data_count, char *json, size_t *json_len)
{
    char values[1000];
    memset(values, 0, sizeof(values));
    int value_len = 0;

    for (int i = 0; i < data_count; i++)
    {
        value_len += snprintf(values + value_len, sizeof(values) - value_len, "%.5f", data[i]);

        if (i != data_count - 1)
        {
            value_len += snprintf(values + value_len, sizeof(values) - value_len, ", ");
        }
    }

    *json_len = sprintf(json, "[%s]", values);
}

void send_data(int client_fd, char *buffer)
{
    time_t start, finish;
    double data[100];
    size_t data_count = 0;

    get_start_finish(buffer, &start, &finish);
    read_many_between_dates(start, finish, data, &data_count);

    char json[2000];
    size_t json_len;
    parse_to_json(data, data_count, json, &json_len);

    const char* response_template =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %d\r\n"
        "\r\n"
        "%s";

    char response[2200];
    size_t response_len = sprintf(response, response_template, json_len, json);

    int total = 0;
    int to_send = (int)response_len;
    
    while (total < to_send)
    {
        int sent = send(client_fd, response + total, response_len - total, 0);
        
        total += sent;
    }
}

DWORD WINAPI start_server_handler(LPVOID arg)
{
    WSADATA wsaData;
    int status = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (status != 0)
    {
        perror("Ошибка инициализации WSA");
        return 1;
    }

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if (server_fd == INVALID_SOCKET)
    {
        fprintf(stderr, "Ошибка создания сокета: %d", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    int opt = 1;

    status = setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        (const char*)&opt,
        sizeof(opt)
    );

    if (status != 0)
    {
        fprintf(stderr, "Ошибка при конфигурировании сокета: %d", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    struct sockaddr_in address;
    int addrlen = sizeof(address);

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;
    
    status = bind(
        server_fd,
        &address,
        addrlen
    );

    if (status == SOCKET_ERROR)
    {
        fprintf(stderr, "Ошибка привязки сокеты: %d", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }
    
    status = listen(
        server_fd,
        1
    );

    if (status == SOCKET_ERROR)
    {
        fprintf(stderr, "Ошибка сокета в прослушиваемое состояние: %d", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }
    
    printf("Начато прослушивание %d порта...", PORT);

    SOCKET client_fd = INVALID_SOCKET;
    char buffer[BUFFER_SIZE];
    
    while (1)
    {
        client_fd = accept(server_fd, &address, &addrlen);

        if (client_fd == INVALID_SOCKET)
        {
            fprintf(stderr, "Ошибка установления связи с клиентом: %d", WSAGetLastError());
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        int received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

        if (received > 0)
        {
            buffer[received] = '\0';
            
            if (strncmp(buffer, "GET /data", 9) == 0)
            {
                send_data(client_fd, buffer);
            }
            else
            {
                const char* response =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 9\r\n"
                    "\r\n"
                    "Not Found";
                send(client_fd, response, (int)strlen(response), 0);
            }
        }

        closesocket(client_fd);
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}

void start_server(void)
{
    _beginthreadex(NULL, 0, start_server_handler, NULL, 0, NULL);
}
