#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "logger.h"
#include "repository.h"
#include "utils.h"

#define RECORDS_PER_SECOND 1
#define BUFFER_SIZE 100000
#define FLUSH_SIZE 5

#define HOURLY_RECORDS_NUM 10
#define DAILY_RECORDS_NUM 100

#define CLEAR_INTERVAL 50
#define CLEAR_HOUR_INTERVAL 15
#define CLEAR_DAY_INTERVAL 5

double buffer[BUFFER_SIZE];
size_t buffer_pointer = 0;

void init_logger(void)
{
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
        double data[FLUSH_SIZE];
        read_last_buffer(data, FLUSH_SIZE);

        save(data, FLUSH_SIZE, time(NULL));

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

        save_hourly(avg, time(NULL));

        return;
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

        save_daily(avg, time(NULL));

        return;
    }
}

void* clear_logger(void* arg)
{
    while (1)
    {
        sleep_(CLEAR_INTERVAL);

        delete_logs_before_date(time(NULL) - CLEAR_INTERVAL);
    }
}

void* clear_hourly_logger(void* arg)
{
    while (1)
    {
        sleep_(CLEAR_HOUR_INTERVAL * HOURLY_RECORDS_NUM);

        delete_logs_before_date(time(NULL) - CLEAR_HOUR_INTERVAL * HOURLY_RECORDS_NUM);
    }
}

void* clear_daily_logger(void* arg)
{
    while (1)
    {
        sleep_(CLEAR_DAY_INTERVAL * DAILY_RECORDS_NUM);

        delete_logs_before_date(time(NULL) - CLEAR_DAY_INTERVAL * DAILY_RECORDS_NUM);
    }
}

void start_hourly_logger(void)
{
    pthread_t thread;

    pthread_create(&thread, NULL, log_hourly_temperature, NULL);
}

void start_daily_logger(void)
{
    pthread_t thread;

    pthread_create(&thread, NULL, log_daily_temperature, NULL);
}

void start_clear_logger(void)
{
    pthread_t thread;

    pthread_create(&thread, NULL, clear_logger, NULL);
}

void start_clear_hourly_logger(void)
{
    pthread_t thread;

    pthread_create(&thread, NULL, clear_hourly_logger, NULL);
}

void start_clear_daily_logger(void)
{
    pthread_t thread;

    pthread_create(&thread, NULL, clear_daily_logger, NULL);
}
