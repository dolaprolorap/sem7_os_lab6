#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "repository.h"
#include <pthread.h>

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

    write(client_fd, response, response_len);
}

void* start_server_handler(void *arg)
{
    int server_fd, client_fd;

    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == 0)
    {
        perror("Ошибка открытия сокета");
        return;
    }

    int opt = 1;
    int status = setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

    if (status < 0)
    {
        perror("Ошибка конфигурации сокета");
        return;
    }

    status = bind(server_fd, &address, addrlen);

    if (status < 0)
    {
        perror("Ошибка привязки сокета");
        return;
    }

    status = listen(server_fd, 1);

    if (status < 0)
    {
        perror("Ошибка прослушивания сокета");
        return;
    }

    printf("Начато прослушивание %d порта...", PORT);

    while (1)
    {
        client_fd = accept(server_fd, &address, &addrlen);

        if (client_fd < 0)
        {
            perror("Ошибка подключения клиента");
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        read(client_fd, buffer, BUFFER_SIZE - 1);

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
            write(client_fd, response, strlen(response));
        }

        close(client_fd);
    }

    close(server_fd);
}

void start_server(void)
{
    pthread_t thread;

    pthread_create(
        &thread,
        NULL,
        start_server_handler,
        NULL
    );
}
