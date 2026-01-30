#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "logger.h"
#include "utils.h"

#define RECORDS_PER_SECOND 1
#define BUFFER_SIZE 100000
#define FLUSH_SIZE 5

#define LOG_FILE "log.txt"
#define HOURLY_LOG_FILE "hourly_log.txt"
#define DAILY_LOG_FILE "daily_log.txt"

#define HOURLY_RECORDS_NUM 10
#define DAILY_RECORDS_NUM 100

#define MAX_RECORDS 10
#define MAX_HOURLY_RECORDS 10
#define MAX_DAILY_RECORDS 2

double buffer[BUFFER_SIZE];
size_t buffer_pointer = 0;

size_t log_size = 0;
size_t hourly_log_size = 0;
size_t daily_log_size = 0;

HANDLE size_mutex;

void init_logger(void)
{
    size_mutex = CreateMutex(NULL, FALSE, NULL);
}

void write_buffer(double data)
{
    if (buffer_pointer >= BUFFER_SIZE)
    {
        buffer_pointer = 0;
    }

    buffer[buffer_pointer++] = data;
}

void read_last_buffer(double *data, size_t size)
{
    if (buffer_pointer - size >= 0)
    {
        memcpy(data, buffer + buffer_pointer - size, size * sizeof(double));

        return;
    }

    int reminder = size - buffer_pointer;

    memcpy(data, buffer + BUFFER_SIZE - reminder, reminder * sizeof(double));
    memcpy(data + reminder, buffer, (size - reminder) * sizeof(double));
}

void log_temperature(double temperature)
{
    write_buffer(temperature);

    if (buffer_pointer != 0 && buffer_pointer % FLUSH_SIZE == 0) 
    {
        FILE *f = fopen(LOG_FILE, "a");

        if (f == NULL)
        {
            perror("Ошибка открытия файла логов");
        }

        double data[FLUSH_SIZE];
        read_last_buffer(data, FLUSH_SIZE);

        char message[100];
        size_t message_len = 0;

        for (int i = 0; i < FLUSH_SIZE; i++)
        {
            message_len += snprintf(message + message_len, sizeof(message) - message_len, "%.5f\n", data[i]);
        }

        fwrite(message, sizeof(char), message_len, f);

        WaitForSingleObject(size_mutex, INFINITE);
        log_size += FLUSH_SIZE;
        ReleaseMutex(size_mutex);

        fclose(f);

        return;
    }
}

void log_hourly_temperature(void)
{
    while (1) 
    {
        sleep_(HOURLY_RECORDS_NUM);

        double data[HOURLY_RECORDS_NUM];

        read_last_buffer(data, HOURLY_RECORDS_NUM);

        double sum = 0;

        for (int i = 0; i < HOURLY_RECORDS_NUM; i++)
        {
            sum += data[i];
        }

        double avg = sum / HOURLY_RECORDS_NUM;

        FILE *file = fopen(HOURLY_LOG_FILE, "a");

        if (file == NULL)
        {
            perror("Ошибка открытия файла логов");
        }

        char message[100];

        size_t len = sprintf(message, "%.5f\n", avg);

        fwrite(message, sizeof(char), len, file);

        WaitForSingleObject(size_mutex, INFINITE);
        hourly_log_size += 1;
        ReleaseMutex(size_mutex);

        fclose(file);
    }
}

void log_daily_temperature(void)
{
    while (1)
    {
        sleep_(DAILY_RECORDS_NUM);

        double data[DAILY_RECORDS_NUM];

        read_last_buffer(data, DAILY_RECORDS_NUM);

        double sum = 0;

        for (int i = 0; i < DAILY_RECORDS_NUM; i++)
        {
            sum += data[i];
        }

        double avg = sum / DAILY_RECORDS_NUM;

        FILE *file = fopen(DAILY_LOG_FILE, "a");

        if (file == NULL)
        {
            perror("Ошибка открытия файла логов");
        }

        char message[100];

        size_t len = sprintf(message, "%.5f\n", avg);

        fwrite(message, sizeof(char), len, file);

        WaitForSingleObject(size_mutex, INFINITE);
        daily_log_size += 1;
        ReleaseMutex(size_mutex);

        fclose(file);
    }
}

void clear_logger_base(int delay, const char* temp_filename, int* records_count, int records_max_count, const char* log_filename)
{
    while(1)
    {
        sleep_(delay);

        FILE *in = fopen(log_filename, "r");
        FILE *out = fopen(temp_filename, "w");
    
        if (!in || !out) {
            perror("Ошибка открытия файла");
            return;
        }
    
        char line[1024];
        int count = 0;
    
        while (fgets(line, sizeof(line), in)) {
            if (count >= *records_count - records_max_count) {
                fputs(line, out);
            }
            count++;
        }

        WaitForSingleObject(size_mutex, INFINITE);
        *records_count = records_max_count;
        ReleaseMutex(size_mutex);

        fclose(in);
        fclose(out);
    
        remove(log_filename);
        rename(temp_filename, log_filename);
    }
}

void* clear_logger(void* arg)
{
    int delay = RECORDS_PER_SECOND * MAX_RECORDS;

    clear_logger_base(delay, ".log.tmp", &log_size, MAX_RECORDS, LOG_FILE);
}

void* clear_hourly_logger(void* arg)
{
    int delay = RECORDS_PER_SECOND * HOURLY_RECORDS_NUM * MAX_HOURLY_RECORDS;

    clear_logger_base(delay, ".hourly_log.tmp", &hourly_log_size, MAX_HOURLY_RECORDS, HOURLY_LOG_FILE);
}

void* clear_daily_logger(void* arg)
{
    int delay = RECORDS_PER_SECOND * DAILY_RECORDS_NUM * MAX_DAILY_RECORDS;

    clear_logger_base(delay, ".daily_log.tmp", &daily_log_size, MAX_DAILY_RECORDS, DAILY_LOG_FILE);
}

void start_hourly_logger(void)
{
    _beginthreadex(NULL, 0, log_hourly_temperature, NULL, 0, NULL);
}

void start_daily_logger(void)
{
    _beginthreadex(NULL, 0, log_daily_temperature, NULL, 0, NULL);
}

void start_clear_logger(void)
{
    _beginthreadex(NULL, 0, clear_logger, NULL, 0, NULL);
}

void start_clear_hourly_logger(void)
{
    _beginthreadex(NULL, 0, clear_hourly_logger, NULL, 0, NULL);
}

void start_clear_daily_logger(void)
{
    _beginthreadex(NULL, 0, clear_daily_logger, NULL, 0, NULL);
}
