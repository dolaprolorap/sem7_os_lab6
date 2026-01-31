#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <stdlib.h>
#include <time.h>

void init_db(void);

void save(double *data, size_t size, time_t time);
void save_hourly(double data, time_t time);
void save_daily(double data, time_t time);

void read_many_between_dates(time_t start, time_t finish, double *data, size_t *data_count);
void read_many_between_dates_hourly(time_t start, time_t finish, double *data, size_t *data_count);
void read_many_between_dates_daily(time_t start, time_t finish, double *data, size_t *data_count);

void delete_logs_before_date(time_t time);
void delete_hourly_logs_before_date(time_t time);
void delete_daily_logs_before_date(time_t time);

#endif // REPOSITORY_H
