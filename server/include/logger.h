#ifndef LOGGER_H
#define LOGGER_H

void init_logger(void);

void log_temperature(double temperature);

void start_hourly_logger(void);
void start_daily_logger(void);

void start_clear_logger(void);
void start_clear_hourly_logger(void);
void start_clear_daily_logger(void);

#endif // LOGGER_H
