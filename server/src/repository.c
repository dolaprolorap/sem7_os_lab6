#include "repository.h"
#include <stdio.h>
#include <sqlite3.h>

#define MAX_READ_DATA 500

sqlite3 *db;

void init_db(void)
{
    sqlite3_open("database.db", &db);

    sqlite3_exec(
        db,
        "CREATE TABLE IF NOT EXISTS data ( \
            value REAL, \
            timestamp INTEGER \
        )",
        NULL,
        NULL,
        NULL
    );

    sqlite3_exec(
        db,
        "CREATE TABLE IF NOT EXISTS hourly_data ( \
            value REAL, \
            timestamp INTEGER \
        )",
        NULL,
        NULL,
        NULL
    );

    sqlite3_exec(
        db,
        "CREATE TABLE IF NOT EXISTS daily_data ( \
            value REAL, \
            timestamp INTEGER \
        )",
        NULL,
        NULL,
        NULL
    );
}

void save(double *data, size_t size, time_t time)
{
    char data_str[300];
    size_t message_len = 0;

    for (int i = 0; i < size; i++)
    {
        message_len += snprintf(data_str + message_len, sizeof(data_str) - message_len, "(%.5f, %d)", data[i], time);

        if (i != size - 1) {
            message_len += snprintf(data_str + message_len, sizeof(data_str) - message_len, ", ");
        }
    }

    printf("%s", data_str);

    char* command[500];

    sprintf(command, "INSERT INTO data (value, timestamp) VALUES %s", data_str);

    sqlite3_exec(
        db,
        command,
        NULL,
        NULL,
        NULL
    );
}

void save_hourly(double data, time_t time)
{
    char* command[500];

    sprintf(command, "INSERT INTO hourly_data VALUES (%.5f, %d)", data, time);

    sqlite3_exec(
        db,
        command,
        NULL,
        NULL,
        NULL
    );
}

void save_daily(double data, time_t time)
{
    char* command[500];

    sprintf(command, "INSERT INTO daily_data VALUES (%.5f, %d)", data, time);

    sqlite3_exec(
        db,
        command,
        NULL,
        NULL,
        NULL
    );
}

void read_many_internal(time_t start, time_t finish, double *data, size_t *data_count, const char* table)
{
    sqlite3_stmt *stmt;

    char command[200];

    snprintf(command, sizeof(command), "SELECT value FROM %s WHERE timestamp >= %d AND timestamp <= %d;", table, start, finish);

    int status = sqlite3_prepare_v2(
        db,
        command,
        -1,
        &stmt,
        NULL
    );

    if (status != SQLITE_OK)
    {
        fprintf(stderr, "Ошибка обращения к БД: %s\n", sqlite3_errmsg(db));
        *data_count = 0;
        return;
    }

    *data_count = 0;

    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
        double value = sqlite3_column_double(stmt, 0);
        data[(*data_count)++] = value;

        if (*data_count >= MAX_READ_DATA) {
            printf("Считано максимальное кол-во отчетов температуры\n");

            break;
        }
    }

    sqlite3_finalize(stmt);
}

void read_many_between_dates(time_t start, time_t finish, double *data, size_t *data_count)
{
    read_many_internal(
        start,
        finish,
        data,
        data_count,
        "data"
    );
}

void read_many_between_dates_hourly(time_t start, time_t finish, double *data, size_t *data_count)
{
    read_many_internal(
        start,
        finish,
        data,
        data_count,
        "hourly_data"
    );
}

void read_many_between_dates_daily(time_t start, time_t finish, double *data, size_t *data_count)
{
    read_many_internal(
        start,
        finish,
        data,
        data_count,
        "daily_data"
    );
}

void delete_internal(time_t time, const char* table)
{
    char command[500];

    sprintf(command, "DELETE FROM %s WHERE timestamp <= %d", table, time);

    int status = sqlite3_exec(
        db,
        command,
        NULL,
        NULL,
        NULL
    );

    if (status != SQLITE_OK)
    {
        fprintf(stderr, "Ошибка удаления из БД: %s\n", sqlite3_errmsg(db));
        return;
    }
}

void delete_logs_before_date(time_t time)
{
    delete_internal(time, "data");
}

void delete_hourly_logs_before_date(time_t time)
{
    delete_internal(time, "hourly_data");
}

void delete_daily_logs_before_date(time_t time)
{
    delete_internal(time, "daily_data");
}
